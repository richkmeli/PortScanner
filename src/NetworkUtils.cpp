#include "NetworkUtils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <stdexcept>
#include <cstring>
#include <regex>

namespace PortScanner {

bool NetworkUtils::is_valid_ipv4(const IPAddress& ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) != 0;
}

IPAddress NetworkUtils::resolve_hostname(const std::string& hostname) {
    struct addrinfo hints{}, *result;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    int status = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
    if (status != 0) {
        throw std::runtime_error("Failed to resolve hostname: " + std::string(gai_strerror(status)));
    }
    
    struct sockaddr_in* addr_in = reinterpret_cast<struct sockaddr_in*>(result->ai_addr);
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr_in->sin_addr), ip_str, INET_ADDRSTRLEN);
    
    freeaddrinfo(result);
    return std::string(ip_str);
}

IPAddress NetworkUtils::get_local_ip() {
    struct ifaddrs *ifaddrs_ptr, *ifa;
    
    if (getifaddrs(&ifaddrs_ptr) == -1) {
        throw std::runtime_error("Failed to get network interfaces");
    }
    
    std::string local_ip;
    for (ifa = ifaddrs_ptr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(addr->sin_addr), ip_str, INET_ADDRSTRLEN);
            
            std::string ip(ip_str);
            // Skip loopback
            if (ip != "127.0.0.1" && std::string(ifa->ifa_name) != "lo") {
                local_ip = ip;
                break;
            }
        }
    }
    
    freeifaddrs(ifaddrs_ptr);
    
    if (local_ip.empty()) {
        return "127.0.0.1";
    }
    
    return local_ip;
}

std::string NetworkUtils::get_service_name(Port port, const std::string& protocol) {
    struct servent* service = getservbyport(htons(port), protocol.c_str());
    if (service != nullptr) {
        return std::string(service->s_name);
    }
    return "unknown";
}

int NetworkUtils::create_tcp_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create TCP socket: " + std::string(strerror(errno)));
    }
    return sockfd;
}

int NetworkUtils::create_udp_socket() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create UDP socket: " + std::string(strerror(errno)));
    }
    return sockfd;
}

int NetworkUtils::create_raw_socket() {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create raw socket (requires root): " + std::string(strerror(errno)));
    }
    
    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        close(sockfd);
        throw std::runtime_error("Failed to set IP_HDRINCL: " + std::string(strerror(errno)));
    }
    
    return sockfd;
}

bool NetworkUtils::set_socket_timeout(int sockfd, Duration timeout) {
    struct timeval tv;
    tv.tv_sec = timeout.count() / 1000;
    tv.tv_usec = (timeout.count() % 1000) * 1000;
    
    return setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == 0 &&
           setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == 0;
}

bool NetworkUtils::set_socket_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) return false;
    
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == 0;
}

sockaddr_in NetworkUtils::create_sockaddr(const IPAddress& ip, Port port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid IP address: " + ip);
    }
    
    return addr;
}

std::string NetworkUtils::sockaddr_to_string(const sockaddr_in& addr) {
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);
    return std::string(ip_str) + ":" + std::to_string(ntohs(addr.sin_port));
}

} // namespace PortScanner