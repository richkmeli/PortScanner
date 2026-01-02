#pragma once

#include "Common.h"
#include <regex>
#include <future>

namespace PortScanner {

class ServiceDetector {
public:
    ServiceDetector();
    
    // Main service detection method
    ServiceInfo detect_service(const IPAddress& target, Port port, 
                              const std::string& banner = "");
    
    // Banner grabbing
    std::string grab_banner(const IPAddress& target, Port port, 
                           Duration timeout = Duration{5000});
    
    // Load custom service patterns
    bool load_patterns_from_file(const std::string& filename);
    
    // Add custom pattern
    void add_pattern(Port port, const ServicePattern& pattern);

private:
    std::unordered_map<Port, std::vector<ServicePattern>> patterns_;
    
    // Protocol-specific banner grabbing
    std::string grab_http_banner(const IPAddress& target, Port port, Duration timeout);
    std::string grab_tcp_banner(const IPAddress& target, Port port, Duration timeout);
    std::string grab_ssl_banner(const IPAddress& target, Port port, Duration timeout);
    
    // Pattern matching
    ServiceInfo match_patterns(Port port, const std::string& banner);
    ServiceInfo analyze_http_response(const std::string& response);
    ServiceInfo analyze_ssh_banner(const std::string& banner);
    ServiceInfo analyze_ftp_banner(const std::string& banner);
    
    // Initialize default patterns
    void init_default_patterns();
};

} // namespace PortScanner