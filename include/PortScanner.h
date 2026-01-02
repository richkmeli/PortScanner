#pragma once

#include "Common.h"
#include "ScanResults.h"
#include "ServiceDetector.h"
#include "AsyncScanner.h"
#include <functional>
#include <future>
#include <memory>

namespace PortScanner {

class PortScanner {
public:
    using ProgressCallback = std::function<void(std::size_t completed, std::size_t total)>;
    
    explicit PortScanner(const ScanConfig& config);
    
    // High-level scanning methods
    ScanResults scan_ports(ProgressCallback progress_cb = nullptr);
    std::future<ScanResults> scan_ports_async(ProgressCallback progress_cb = nullptr);
    
    // Single port scanning
    ScanResult scan_single_port(Port port, ScanType scan_type = ScanType::TCP_CONNECT);
    
    // Configuration management
    void update_config(const ScanConfig& config);
    const ScanConfig& get_config() const { return config_; }
    
    // Performance mode selection
    void set_performance_mode(bool high_performance = true);
    
    // IPv6 support
    static bool is_ipv6_address(const IPAddress& ip);
    
    // Cancel ongoing scan
    void cancel_scan();

private:
    ScanConfig config_;
    std::unique_ptr<ServiceDetector> service_detector_;
    std::unique_ptr<AsyncScanner> async_scanner_;
    bool high_performance_mode_ = false;
    
    // Legacy scanning methods (for compatibility)
    ScanResult tcp_connect_scan(Port port);
    ScanResult tcp_syn_scan(Port port);
    ScanResult udp_scan(Port port);
    ScanResult tcp_ack_scan(Port port);
    ScanResult tcp_fin_scan(Port port);
    
    // Helper methods
    bool is_valid_ip(const IPAddress& ip);
    void init_components();
};

} // namespace PortScanner