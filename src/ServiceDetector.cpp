#include "ServiceDetector.h"
#include "NetworkUtils.h"
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <algorithm>

namespace PortScanner {

// Default service patterns
const std::unordered_map<Port, std::vector<ServicePattern>> SERVICE_PATTERNS = {
    {22, {{"SSH-", "ssh", R"(SSH-([0-9\.]+))", 0.9f}}},
    {21, {{"220", "ftp", R"(220.*?([A-Za-z0-9\.]+))", 0.8f}}},
    {80, {{"HTTP/", "http", R"(Server:\s*([^\r\n]+))", 0.9f}}},
    {443, {{"HTTP/", "https", R"(Server:\s*([^\r\n]+))", 0.9f}}},
    {25, {{"220", "smtp", R"(220\s+([^\s]+))", 0.8f}}},
    {53, {{"", "dns", "", 0.7f}}},
    {110, {{"+OK", "pop3", R"(\+OK\s+([^\r\n]+))", 0.8f}}},
    {143, {{"* OK", "imap", R"(\*\s+OK\s+([^\r\n]+))", 0.8f}}},
    {3306, {{"", "mysql", R"(([0-9\.]+))", 0.7f}}},
    {5432, {{"", "postgresql", "", 0.7f}}},
    {6379, {{"", "redis", "", 0.7f}}},
    {27017, {{"", "mongodb", "", 0.7f}}}
};

ServiceDetector::ServiceDetector() {
    init_default_patterns();
}

ServiceInfo ServiceDetector::detect_service(const IPAddress& target, Port port, const std::string& banner) {
    std::string service_banner = banner;
    
    if (service_banner.empty()) {
        service_banner = grab_banner(target, port);
    }
    
    ServiceInfo info = match_patterns(port, service_banner);
    
    // Enhanced detection for specific protocols
    if (port == 80 || port == 8080 || port == 443) {
        auto http_info = analyze_http_response(service_banner);
        if (http_info.confidence > info.confidence) {
            info = http_info;
        }
    } else if (port == 22) {
        auto ssh_info = analyze_ssh_banner(service_banner);
        if (ssh_info.confidence > info.confidence) {
            info = ssh_info;
        }
    } else if (port == 21) {
        auto ftp_info = analyze_ftp_banner(service_banner);
        if (ftp_info.confidence > info.confidence) {
            info = ftp_info;
        }
    }
    
    return info;
}

std::string ServiceDetector::grab_banner(const IPAddress& target, Port port, Duration timeout) {
    // Try different banner grabbing methods based on port
    if (port == 80 || port == 8080) {
        return grab_http_banner(target, port, timeout);
    } else if (port == 443) {
        return grab_ssl_banner(target, port, timeout);
    } else {
        return grab_tcp_banner(target, port, timeout);
    }
}

std::string ServiceDetector::grab_http_banner(const IPAddress& target, Port port, Duration timeout) {
    try {
        int sockfd = NetworkUtils::create_tcp_socket();
        NetworkUtils::set_socket_timeout(sockfd, timeout);
        
        sockaddr_in addr = NetworkUtils::create_sockaddr(target, port);
        
        if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0) {
            std::string http_request = "GET / HTTP/1.1\r\nHost: " + target + "\r\nConnection: close\r\n\r\n";
            send(sockfd, http_request.c_str(), http_request.length(), 0);
            
            char buffer[4096] = {0};
            ssize_t received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            
            close(sockfd);
            
            if (received > 0) {
                return std::string(buffer, received);
            }
        }
        
        close(sockfd);
    } catch (const std::exception&) {
        // Fallback to basic TCP banner grab
    }
    
    return grab_tcp_banner(target, port, timeout);
}

std::string ServiceDetector::grab_tcp_banner(const IPAddress& target, Port port, Duration timeout) {
    try {
        int sockfd = NetworkUtils::create_tcp_socket();
        NetworkUtils::set_socket_timeout(sockfd, timeout);
        
        sockaddr_in addr = NetworkUtils::create_sockaddr(target, port);
        
        if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0) {
            char buffer[1024] = {0};
            ssize_t received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            
            close(sockfd);
            
            if (received > 0) {
                return std::string(buffer, received);
            }
        }
        
        close(sockfd);
    } catch (const std::exception&) {
        // Connection failed
    }
    
    return "";
}

std::string ServiceDetector::grab_ssl_banner(const IPAddress& target, Port port, Duration timeout) {
    // Basic SSL banner grabbing - would need OpenSSL for full implementation
    return grab_tcp_banner(target, port, timeout);
}

ServiceInfo ServiceDetector::match_patterns(Port port, const std::string& banner) {
    ServiceInfo info;
    
    auto it = patterns_.find(port);
    if (it != patterns_.end()) {
        for (const auto& pattern : it->second) {
            if (banner.find(pattern.pattern) != std::string::npos) {
                info.name = pattern.service_name;
                info.confidence = pattern.confidence;
                
                // Simple version extraction (without regex for now)
                if (!pattern.version_regex.empty() && pattern.pattern == "SSH-") {
                    std::size_t pos = banner.find("SSH-");
                    if (pos != std::string::npos) {
                        std::size_t end = banner.find(' ', pos);
                        if (end == std::string::npos) end = banner.find('\r', pos);
                        if (end == std::string::npos) end = banner.find('\n', pos);
                        if (end != std::string::npos) {
                            info.version = banner.substr(pos + 4, end - pos - 4);
                        }
                    }
                }
                
                break;
            }
        }
    }
    
    // Fallback to common port services
    if (info.name.empty()) {
        info.name = NetworkUtils::get_service_name(port);
        info.confidence = 0.5f;
    }
    
    return info;
}

ServiceInfo ServiceDetector::analyze_http_response(const std::string& response) {
    ServiceInfo info;
    info.name = "http";
    info.confidence = 0.8f;
    
    // Extract server information (simple string search)
    std::size_t server_pos = response.find("Server:");
    if (server_pos != std::string::npos) {
        std::size_t start = server_pos + 8; // "Server: ".length()
        std::size_t end = response.find('\r', start);
        if (end == std::string::npos) end = response.find('\n', start);
        if (end != std::string::npos) {
            info.product = response.substr(start, end - start);
            info.confidence = 0.9f;
        }
    }
    
    // Check for HTTPS
    if (response.find("HTTP/1.1") != std::string::npos || response.find("HTTP/2") != std::string::npos) {
        info.version = "1.1";
    }
    
    return info;
}

ServiceInfo ServiceDetector::analyze_ssh_banner(const std::string& banner) {
    ServiceInfo info;
    info.name = "ssh";
    info.confidence = 0.9f;
    
    // Simple SSH version extraction
    std::size_t ssh_pos = banner.find("SSH-");
    if (ssh_pos != std::string::npos) {
        std::size_t version_start = ssh_pos + 4;
        std::size_t version_end = banner.find('-', version_start);
        if (version_end != std::string::npos) {
            info.version = banner.substr(version_start, version_end - version_start);
            
            std::size_t product_start = version_end + 1;
            std::size_t product_end = banner.find(' ', product_start);
            if (product_end == std::string::npos) product_end = banner.find('\r', product_start);
            if (product_end == std::string::npos) product_end = banner.find('\n', product_start);
            if (product_end != std::string::npos) {
                info.product = banner.substr(product_start, product_end - product_start);
            }
            info.confidence = 0.95f;
        }
    }
    
    return info;
}

ServiceInfo ServiceDetector::analyze_ftp_banner(const std::string& banner) {
    ServiceInfo info;
    info.name = "ftp";
    info.confidence = 0.8f;
    
    // Simple FTP banner analysis
    if (banner.find("220") == 0) {
        std::size_t start = 4; // Skip "220 "
        std::size_t end = banner.find('\r', start);
        if (end == std::string::npos) end = banner.find('\n', start);
        if (end != std::string::npos) {
            info.product = banner.substr(start, end - start);
            info.confidence = 0.85f;
        }
    }
    
    return info;
}

void ServiceDetector::init_default_patterns() {
    patterns_ = SERVICE_PATTERNS;
}

bool ServiceDetector::load_patterns_from_file(const std::string&) {
    // Implementation would load custom patterns from file
    // For now, return false to indicate not implemented
    return false;
}

void ServiceDetector::add_pattern(Port port, const ServicePattern& pattern) {
    patterns_[port].push_back(pattern);
}

} // namespace PortScanner