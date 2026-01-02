#include "PortScanner.h"
#include "NetworkUtils.h"
#include "ConfigManager.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <poll.h>
#include <cstring>

namespace PortScanner {

PortScanner::PortScanner(const ScanConfig& config) : config_(config) {
    init_components();
}

void PortScanner::init_components() {
    service_detector_ = std::make_unique<ServiceDetector>();
    
    if (high_performance_mode_) {
        async_scanner_ = std::make_unique<AsyncScanner>(config_);
    }
}

ScanResults PortScanner::scan_ports(ProgressCallback progress_cb) {
    if (high_performance_mode_ && async_scanner_) {
        auto future_result = async_scanner_->scan_async(progress_cb);
        return future_result.get();
    }
    
    // Fallback to traditional multi-threaded scanning
    ScanResults results;
    std::mutex results_mutex;
    std::atomic<std::size_t> completed{0};
    
    const std::size_t thread_count = std::min(config_.thread_count, config_.ports.size());
    const std::size_t ports_per_thread = (config_.ports.size() + thread_count - 1) / thread_count;
    
    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    
    for (std::size_t i = 0; i < thread_count; ++i) {
        std::size_t start_idx = i * ports_per_thread;
        std::size_t end_idx = std::min(start_idx + ports_per_thread, config_.ports.size());
        
        if (start_idx >= config_.ports.size()) break;
        
        threads.emplace_back([this, &results, &results_mutex, &completed, 
                            start_idx, end_idx, progress_cb]() {
            
            for (std::size_t idx = start_idx; idx < end_idx; ++idx) {
                try {
                    ScanResult result = scan_single_port(config_.ports[idx], config_.scan_type);
                    
                    {
                        std::lock_guard<std::mutex> lock(results_mutex);
                        results.add_result(result);
                    }
                    
                    std::size_t current_completed = completed.fetch_add(1) + 1;
                    
                    if (progress_cb) {
                        progress_cb(current_completed, config_.ports.size());
                    }
                    
                } catch (const std::exception&) {
                    std::lock_guard<std::mutex> lock(results_mutex);
                    results.add_result(config_.ports[idx], PortStatus::UNKNOWN);
                    completed.fetch_add(1);
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    return results;
}

std::future<ScanResults> PortScanner::scan_ports_async(ProgressCallback progress_cb) {
    if (high_performance_mode_ && async_scanner_) {
        return async_scanner_->scan_async(progress_cb);
    }
    
    return std::async(std::launch::async, [this, progress_cb]() {
        return scan_ports(progress_cb);
    });
}

ScanResult PortScanner::scan_single_port(Port port, ScanType scan_type) {
    switch (scan_type) {
        case ScanType::TCP_CONNECT:
            return tcp_connect_scan(port);
        case ScanType::TCP_SYN:
            return tcp_syn_scan(port);
        case ScanType::UDP:
            return udp_scan(port);
        case ScanType::TCP_ACK:
            return tcp_ack_scan(port);
        case ScanType::TCP_FIN:
            return tcp_fin_scan(port);
        default:
            throw std::runtime_error("Unsupported scan type");
    }
}

void PortScanner::update_config(const ScanConfig& config) {
    config_ = config;
    init_components();
}

void PortScanner::set_performance_mode(bool high_performance) {
    high_performance_mode_ = high_performance;
    init_components();
}

bool PortScanner::is_ipv6_address(const IPAddress& ip) {
    return ip.find(':') != std::string::npos;
}

void PortScanner::cancel_scan() {
    if (async_scanner_) {
        async_scanner_->cancel();
    }
}

ScanResult PortScanner::tcp_connect_scan(Port port) {
    auto start_time = std::chrono::steady_clock::now();
    
    int sockfd = NetworkUtils::create_tcp_socket();
    NetworkUtils::set_socket_timeout(sockfd, config_.timeout);
    
    sockaddr_in target_addr = NetworkUtils::create_sockaddr(config_.target, port);
    
    int result = connect(sockfd, reinterpret_cast<struct sockaddr*>(&target_addr), sizeof(target_addr));
    
    auto end_time = std::chrono::steady_clock::now();
    auto response_time = std::chrono::duration_cast<Duration>(end_time - start_time);
    
    close(sockfd);
    
    ScanResult scan_result;
    scan_result.port = port;
    scan_result.status = (result == 0) ? PortStatus::OPEN : PortStatus::CLOSED;
    scan_result.response_time = response_time;
    scan_result.ip_version = is_ipv6_address(config_.target) ? IPVersion::IPv6 : IPVersion::IPv4;
    
    // Enhanced service detection
    if (scan_result.status == PortStatus::OPEN && config_.service_detection && service_detector_) {
        scan_result.service = service_detector_->detect_service(config_.target, port);
        
        if (config_.banner_grabbing) {
            scan_result.banner = service_detector_->grab_banner(config_.target, port, Duration{2000});
        }
    }
    
    return scan_result;
}

ScanResult PortScanner::tcp_syn_scan(Port port) {
    try {
        int sockfd = NetworkUtils::create_raw_socket();
        close(sockfd);
        
        // For now, fallback to connect scan
        return tcp_connect_scan(port);
        
    } catch (const std::exception&) {
        return tcp_connect_scan(port);
    }
}

ScanResult PortScanner::udp_scan(Port port) {
    auto start_time = std::chrono::steady_clock::now();
    
    int sockfd = NetworkUtils::create_udp_socket();
    NetworkUtils::set_socket_timeout(sockfd, config_.timeout);
    
    sockaddr_in target_addr = NetworkUtils::create_sockaddr(config_.target, port);
    
    const char* test_data = "test";
    ssize_t sent = sendto(sockfd, test_data, strlen(test_data), 0,
                         reinterpret_cast<struct sockaddr*>(&target_addr), sizeof(target_addr));
    
    PortStatus status = PortStatus::UNKNOWN;
    
    if (sent > 0) {
        char buffer[1024];
        struct pollfd pfd;
        pfd.fd = sockfd;
        pfd.events = POLLIN;
        
        int poll_result = poll(&pfd, 1, config_.timeout.count());
        
        if (poll_result > 0) {
            recv(sockfd, buffer, sizeof(buffer), 0);
            status = PortStatus::OPEN;
        } else if (poll_result == 0) {
            status = PortStatus::FILTERED;
        } else {
            status = PortStatus::CLOSED;
        }
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto response_time = std::chrono::duration_cast<Duration>(end_time - start_time);
    
    close(sockfd);
    
    ScanResult scan_result;
    scan_result.port = port;
    scan_result.status = status;
    scan_result.response_time = response_time;
    scan_result.ip_version = is_ipv6_address(config_.target) ? IPVersion::IPv6 : IPVersion::IPv4;
    
    if (status == PortStatus::OPEN && config_.service_detection && service_detector_) {
        scan_result.service = service_detector_->detect_service(config_.target, port);
    }
    
    return scan_result;
}

ScanResult PortScanner::tcp_ack_scan(Port port) {
    // ACK scan implementation would require raw sockets
    // For now, fallback to connect scan
    return tcp_connect_scan(port);
}

ScanResult PortScanner::tcp_fin_scan(Port port) {
    // FIN scan implementation would require raw sockets
    // For now, fallback to connect scan
    return tcp_connect_scan(port);
}

bool PortScanner::is_valid_ip(const IPAddress& ip) {
    return NetworkUtils::is_valid_ipv4(ip) || is_ipv6_address(ip);
}

} // namespace PortScanner