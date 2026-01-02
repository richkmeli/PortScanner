#include "AsyncScanner.h"
#include "NetworkUtils.h"
#include "ServiceDetector.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <thread>

namespace PortScanner {

AsyncScanner::AsyncScanner(const ScanConfig& config) : config_(config), epoll_fd_(-1) {
    setup_epoll();
}

AsyncScanner::~AsyncScanner() {
    cleanup_epoll();
}

std::future<ScanResults> AsyncScanner::scan_async(ProgressCallback progress_cb) {
    return std::async(std::launch::async, [this, progress_cb]() {
        ScanResults results;
        cancelled_.store(false);
        completed_ports_.store(0);
        open_ports_.store(0);
        
        auto start_time = std::chrono::steady_clock::now();
        
        try {
            // Create connections in batches for better performance
            const std::size_t batch_size = std::min(config_.thread_count, config_.ports.size());
            
            for (std::size_t i = 0; i < config_.ports.size() && !cancelled_.load(); i += batch_size) {
                std::size_t end = std::min(i + batch_size, config_.ports.size());
                std::vector<Port> batch_ports(config_.ports.begin() + i, config_.ports.begin() + end);
                
                if (create_connections(batch_ports)) {
                    process_events(results, progress_cb);
                }
                
                // Clean up connections for next batch
                for (auto& conn : connections_) {
                    if (conn.sockfd >= 0) {
                        close(conn.sockfd);
                    }
                }
                connections_.clear();
                fd_to_connection_.clear();
            }
            
        } catch (const std::exception& e) {
            // Handle errors gracefully
        }
        
        return results;
    });
}

void AsyncScanner::cancel() {
    cancelled_.store(true);
}

AsyncScanner::ScanStats AsyncScanner::get_stats() const {
    ScanStats stats;
    stats.total_ports = config_.ports.size();
    stats.completed_ports = completed_ports_.load();
    stats.open_ports = open_ports_.load();
    stats.active_connections = connections_.size();
    
    // Calculate ports per second (simplified)
    if (stats.completed_ports > 0) {
        stats.ports_per_second = static_cast<float>(stats.completed_ports) / 
                                (stats.elapsed_time.count() / 1000.0f);
    }
    
    return stats;
}

void AsyncScanner::setup_epoll() {
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ < 0) {
        throw std::runtime_error("Failed to create epoll instance");
    }
}

void AsyncScanner::cleanup_epoll() {
    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
    
    for (auto& conn : connections_) {
        if (conn.sockfd >= 0) {
            close(conn.sockfd);
        }
    }
    connections_.clear();
    fd_to_connection_.clear();
}

bool AsyncScanner::create_connections(const std::vector<Port>& ports) {
    connections_.reserve(ports.size());
    
    for (Port port : ports) {
        if (cancelled_.load()) break;
        
        try {
            int sockfd = create_socket_for_target();
            if (sockfd < 0) continue;
            
            set_socket_options(sockfd);
            NetworkUtils::set_socket_nonblocking(sockfd);
            
            Connection conn;
            conn.sockfd = sockfd;
            conn.port = port;
            conn.start_time = std::chrono::steady_clock::now();
            
            // Add to epoll
            epoll_event event;
            event.events = EPOLLOUT | EPOLLET;
            event.data.fd = sockfd;
            
            if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, sockfd, &event) == 0) {
                std::size_t conn_idx = connections_.size();
                connections_.push_back(conn);
                fd_to_connection_[sockfd] = conn_idx;
                
                // Start non-blocking connect
                sockaddr_in addr = NetworkUtils::create_sockaddr(config_.target, port);
                connect(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
            } else {
                close(sockfd);
            }
            
        } catch (const std::exception&) {
            // Skip this port on error
        }
    }
    
    return !connections_.empty();
}

void AsyncScanner::process_events(ScanResults& results, ProgressCallback progress_cb) {
    const int max_events = 1000;
    epoll_event events[max_events];
    
    auto timeout_ms = static_cast<int>(config_.timeout.count());
    std::size_t processed = 0;
    
    while (processed < connections_.size() && !cancelled_.load()) {
        int event_count = epoll_wait(epoll_fd_, events, max_events, timeout_ms);
        
        if (event_count < 0) break;
        
        if (event_count == 0) {
            // Timeout - mark remaining connections as closed/filtered
            for (auto& conn : connections_) {
                if (!conn.connected) {
                    auto end_time = std::chrono::steady_clock::now();
                    auto response_time = std::chrono::duration_cast<Duration>(end_time - conn.start_time);
                    
                    ScanResult result;
                    result.port = conn.port;
                    result.status = PortStatus::FILTERED;
                    result.response_time = response_time;
                    result.ip_version = is_ipv6_address(config_.target) ? IPVersion::IPv6 : IPVersion::IPv4;
                    
                    results.add_result(result);
                    processed++;
                }
            }
            break;
        }
        
        for (int i = 0; i < event_count; ++i) {
            handle_connection_event(events[i], results);
            processed++;
            
            if (progress_cb) {
                progress_cb(completed_ports_.load(), config_.ports.size());
            }
        }
    }
}

void AsyncScanner::handle_connection_event(const epoll_event& event, ScanResults& results) {
    int sockfd = event.data.fd;
    auto it = fd_to_connection_.find(sockfd);
    
    if (it == fd_to_connection_.end()) return;
    
    Connection& conn = connections_[it->second];
    auto end_time = std::chrono::steady_clock::now();
    auto response_time = std::chrono::duration_cast<Duration>(end_time - conn.start_time);
    
    ScanResult result;
    result.port = conn.port;
    result.response_time = response_time;
    result.ip_version = is_ipv6_address(config_.target) ? IPVersion::IPv6 : IPVersion::IPv4;
    
    if (event.events & EPOLLOUT) {
        // Connection attempt completed
        int error = 0;
        socklen_t len = sizeof(error);
        
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) == 0 && error == 0) {
            // Connection successful
            result.status = PortStatus::OPEN;
            conn.connected = true;
            open_ports_.fetch_add(1);
            
            // Perform service detection if enabled
            if (config_.service_detection) {
                ServiceDetector detector;
                result.service = detector.detect_service(config_.target, conn.port);
                
                if (config_.banner_grabbing) {
                    result.banner = detector.grab_banner(config_.target, conn.port, Duration{2000});
                }
            }
        } else {
            // Connection failed
            result.status = PortStatus::CLOSED;
        }
    } else if (event.events & (EPOLLERR | EPOLLHUP)) {
        // Connection error
        result.status = PortStatus::CLOSED;
    }
    
    results.add_result(result);
    completed_ports_.fetch_add(1);
    
    // Remove from epoll and close socket
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, sockfd, nullptr);
    close(sockfd);
    conn.sockfd = -1;
}

int AsyncScanner::create_socket_for_target() {
    if (is_ipv6_address(config_.target)) {
        return socket(AF_INET6, SOCK_STREAM, 0);
    } else {
        return socket(AF_INET, SOCK_STREAM, 0);
    }
}

bool AsyncScanner::is_ipv6_address(const IPAddress& ip) {
    return ip.find(':') != std::string::npos;
}

void AsyncScanner::set_socket_options(int sockfd) {
    // Set socket options for optimal performance
    int flag = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    
    // Set TCP_NODELAY for faster connection establishment
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
    
    // Set socket timeout
    struct timeval timeout;
    timeout.tv_sec = config_.timeout.count() / 1000;
    timeout.tv_usec = (config_.timeout.count() % 1000) * 1000;
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
}

} // namespace PortScanner