#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <unordered_map>

namespace PortScanner {

// Type aliases for better readability
using Port = std::uint16_t;
using IPAddress = std::string;
using Duration = std::chrono::milliseconds;

// Constants
constexpr Port MIN_PORT = 1;
constexpr Port MAX_PORT = 65535;
constexpr Duration DEFAULT_TIMEOUT{3000};
constexpr std::size_t DEFAULT_THREAD_COUNT = 100;

// IP version support
enum class IPVersion {
    IPv4,
    IPv6,
    AUTO
};

// Scan types
enum class ScanType {
    TCP_CONNECT,
    TCP_SYN,
    UDP,
    TCP_ACK,
    TCP_FIN
};

// Port status
enum class PortStatus {
    OPEN,
    CLOSED,
    FILTERED,
    UNKNOWN,
    OPEN_FILTERED
};

// Service information structure
struct ServiceInfo {
    std::string name;
    std::string version;
    std::string product;
    std::string extra_info;
    float confidence = 0.0f;
};

// Result structure with enhanced service detection
struct ScanResult {
    Port port;
    PortStatus status;
    Duration response_time;
    ServiceInfo service;
    std::string banner;
    IPVersion ip_version = IPVersion::IPv4;
};

// Configuration structure for advanced features
struct ScanConfig {
    IPAddress target;
    std::vector<Port> ports;
    ScanType scan_type = ScanType::TCP_CONNECT;
    IPVersion ip_version = IPVersion::AUTO;
    Duration timeout = DEFAULT_TIMEOUT;
    std::size_t thread_count = DEFAULT_THREAD_COUNT;
    bool verbose = false;
    bool service_detection = true;
    bool banner_grabbing = true;
    std::string config_file;
    std::string output_format = "txt";
    std::string output_file;
};

// Service detection patterns
struct ServicePattern {
    std::string pattern;
    std::string service_name;
    std::string version_regex;
    float confidence;
};

// Common service patterns for detection
extern const std::unordered_map<Port, std::vector<ServicePattern>> SERVICE_PATTERNS;

} // namespace PortScanner