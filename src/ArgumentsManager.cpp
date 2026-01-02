#include "ArgumentsManager.h"
#include "NetworkUtils.h"
#include "ConfigManager.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <getopt.h>

namespace PortScanner {

ArgumentsManager::ArgumentsManager(int argc, char* argv[]) {
    try {
        parse_arguments(argc, argv);
        if (!should_exit_) {
            validate_config();
        }
    } catch (const std::exception& e) {
        throw ArgumentError(e.what());
    }
}

void ArgumentsManager::parse_arguments(int argc, char* argv[]) {
    const struct option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"version", no_argument, nullptr, 'V'},
        {"verbose", no_argument, nullptr, 'v'},
        {"target", required_argument, nullptr, 't'},
        {"ports", required_argument, nullptr, 'p'},
        {"timeout", required_argument, nullptr, 'T'},
        {"threads", required_argument, nullptr, 'j'},
        {"scan-type", required_argument, nullptr, 's'},
        {"ipv6", no_argument, nullptr, '6'},
        {"config", required_argument, nullptr, 'c'},
        {"output", required_argument, nullptr, 'o'},
        {"format", required_argument, nullptr, 'f'},
        {"no-service-detection", no_argument, nullptr, 'S'},
        {"no-banner-grab", no_argument, nullptr, 'B'},
        {"performance", no_argument, nullptr, 'P'},
        {nullptr, 0, nullptr, 0}
    };
    
    // Initialize with defaults
    config_ = ConfigManager::create_default_config();
    
    int opt;
    while ((opt = getopt_long(argc, argv, "hVvt:p:T:j:s:6c:o:f:SBP", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'h':
                print_help();
                should_exit_ = true;
                return;
                
            case 'V':
                print_version();
                should_exit_ = true;
                return;
                
            case 'v':
                config_.verbose = true;
                break;
                
            case 't':
                config_.target = optarg;
                break;
                
            case 'p':
                config_.ports = parse_port_range(optarg);
                break;
                
            case 'T':
                config_.timeout = Duration{std::stoi(optarg)};
                break;
                
            case 'j':
                config_.thread_count = std::stoul(optarg);
                break;
                
            case 's': {
                config_.scan_type = ConfigManager::string_to_scan_type(optarg);
                break;
            }
            
            case '6':
                config_.ip_version = IPVersion::IPv6;
                break;
                
            case 'c':
                config_.config_file = optarg;
                break;
                
            case 'o':
                config_.output_file = optarg;
                break;
                
            case 'f':
                config_.output_format = optarg;
                break;
                
            case 'S':
                config_.service_detection = false;
                break;
                
            case 'B':
                config_.banner_grabbing = false;
                break;
                
            case 'P':
                // Performance mode will be handled in main
                break;
                
            default:
                throw ArgumentError("Invalid option");
        }
    }
    
    // Handle positional arguments
    if (optind < argc && config_.target == "127.0.0.1") {
        config_.target = argv[optind];
    }
}

void ArgumentsManager::validate_config() {
    // Validate IP address or resolve hostname
    if (!NetworkUtils::is_valid_ipv4(config_.target) && config_.ip_version != IPVersion::IPv6) {
        try {
            config_.target = NetworkUtils::resolve_hostname(config_.target);
        } catch (const std::exception&) {
            throw ArgumentError("Invalid IP address or hostname: " + config_.target);
        }
    }
    
    // Validate timeout
    if (config_.timeout.count() <= 0 || config_.timeout.count() > 60000) {
        throw ArgumentError("Timeout must be between 1 and 60000 milliseconds");
    }
    
    // Validate thread count
    if (config_.thread_count == 0 || config_.thread_count > 2000) {
        throw ArgumentError("Thread count must be between 1 and 2000");
    }
    
    // Validate ports
    if (config_.ports.empty()) {
        throw ArgumentError("No ports specified");
    }
    
    for (Port port : config_.ports) {
        if (port < MIN_PORT || port > MAX_PORT) {
            throw ArgumentError("Port " + std::to_string(port) + " is out of valid range");
        }
    }
    
    // Validate output format
    if (config_.output_format != "txt" && config_.output_format != "json" && config_.output_format != "xml") {
        throw ArgumentError("Invalid output format. Supported: txt, json, xml");
    }
}

std::vector<Port> ArgumentsManager::parse_port_range(const std::string& port_str) {
    std::vector<Port> ports;
    std::istringstream iss(port_str);
    std::string token;
    
    while (std::getline(iss, token, ',')) {
        // Remove whitespace
        token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());
        
        if (token.find('-') != std::string::npos) {
            // Range format: start-end
            std::size_t dash_pos = token.find('-');
            Port start = std::stoi(token.substr(0, dash_pos));
            Port end = std::stoi(token.substr(dash_pos + 1));
            
            if (start > end) {
                std::swap(start, end);
            }
            
            for (Port p = start; p <= end; ++p) {
                ports.push_back(p);
            }
        } else {
            // Single port
            ports.push_back(std::stoi(token));
        }
    }
    
    // Remove duplicates and sort
    std::sort(ports.begin(), ports.end());
    ports.erase(std::unique(ports.begin(), ports.end()), ports.end());
    
    return ports;
}

void ArgumentsManager::print_help() {
    std::cout << R"(PortScanner v2.1.0 - Advanced C++ Port Scanner

USAGE:
    PortScanner [OPTIONS] [TARGET]

OPTIONS:
    -h, --help                  Show this help message
    -V, --version               Show version information
    -v, --verbose               Enable verbose output
    -t, --target <IP>           Target IP address or hostname
    -p, --ports <PORTS>         Port specification (e.g., 80,443,1000-2000)
    -T, --timeout <MS>          Timeout in milliseconds (default: 3000)
    -j, --threads <N>           Number of threads (default: 100, max: 2000)
    -s, --scan-type <TYPE>      Scan type: tcp, syn, udp, ack, fin (default: tcp)
    -6, --ipv6                  Force IPv6 scanning
    -c, --config <FILE>         Load configuration from file (JSON/XML)
    -o, --output <FILE>         Output file path
    -f, --format <FORMAT>       Output format: txt, json, xml (default: txt)
    -S, --no-service-detection  Disable service detection
    -B, --no-banner-grab        Disable banner grabbing
    -P, --performance           Enable high-performance mode

EXAMPLES:
    PortScanner 192.168.1.1
    PortScanner -p 80,443,8080 -t google.com
    PortScanner -p 1-1000 -j 500 -T 5000 192.168.1.1
    PortScanner -s syn -p 22,80,443 -v example.com
    PortScanner -c config.json -o results.xml -f xml
    PortScanner -P -j 1000 -p 1-65535 target.com

ADVANCED FEATURES:
    - IPv6 support with automatic detection
    - Advanced service detection and banner grabbing
    - High-performance async I/O scanning
    - Configuration file support (JSON/XML)
    - Multiple output formats with detailed reporting
    - Enhanced scan types (ACK, FIN scans)

NOTES:
    - SYN, ACK, FIN scans require root privileges
    - High-performance mode uses async I/O for better speed
    - Configuration files allow complex scan setups
    - Results are automatically saved for successful scans
)";
}

void ArgumentsManager::print_version() {
    std::cout << "PortScanner v2.1.0 - Advanced Edition\n";
    std::cout << "Built with modern C++17 and advanced features\n";
    std::cout << "Features: IPv6, Async I/O, Service Detection, Config Files\n";
    std::cout << "Copyright (c) 2024\n";
}

} // namespace PortScanner