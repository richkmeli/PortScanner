#pragma once

#include "Common.h"
#include "ScanResults.h"
#include <sys/epoll.h>
#include <future>
#include <atomic>

namespace PortScanner {

class AsyncScanner {
public:
    using ProgressCallback = std::function<void(std::size_t completed, std::size_t total)>;
    
    explicit AsyncScanner(const ScanConfig& config);
    ~AsyncScanner();
    
    // High-performance async scanning
    std::future<ScanResults> scan_async(ProgressCallback progress_cb = nullptr);
    
    // Cancel ongoing scan
    void cancel();
    
    // Get current scan statistics
    struct ScanStats {
        std::size_t total_ports;
        std::size_t completed_ports;
        std::size_t open_ports;
        std::size_t active_connections;
        Duration elapsed_time;
        float ports_per_second;
    };
    
    ScanStats get_stats() const;

private:
    ScanConfig config_;
    int epoll_fd_;
    std::atomic<bool> cancelled_{false};
    std::atomic<std::size_t> completed_ports_{0};
    std::atomic<std::size_t> open_ports_{0};
    
    // Connection management
    struct Connection {
        int sockfd;
        Port port;
        std::chrono::steady_clock::time_point start_time;
        bool connected = false;
    };
    
    std::vector<Connection> connections_;
    std::unordered_map<int, std::size_t> fd_to_connection_;
    
    // Core async methods
    void setup_epoll();
    void cleanup_epoll();
    
    bool create_connections(const std::vector<Port>& ports);
    void process_events(ScanResults& results, ProgressCallback progress_cb);
    void handle_connection_event(const epoll_event& event, ScanResults& results);
    
    // IPv6 support
    int create_socket_for_target();
    bool is_ipv6_address(const IPAddress& ip);
    
    // Performance optimizations
    void set_socket_options(int sockfd);
    void batch_connect(std::size_t batch_size = 1000);
};

} // namespace PortScanner