#pragma once

#include "Common.h"
#include <sys/socket.h>
#include <netinet/in.h>

namespace PortScanner {

class NetworkUtils {
public:
    static bool is_valid_ipv4(const IPAddress& ip);
    static IPAddress resolve_hostname(const std::string& hostname);
    static IPAddress get_local_ip();
    static std::string get_service_name(Port port, const std::string& protocol = "tcp");
    
    // Socket utilities
    static int create_tcp_socket();
    static int create_udp_socket();
    static int create_raw_socket();
    
    static bool set_socket_timeout(int sockfd, Duration timeout);
    static bool set_socket_nonblocking(int sockfd);
    
    // Address utilities
    static sockaddr_in create_sockaddr(const IPAddress& ip, Port port);
    static std::string sockaddr_to_string(const sockaddr_in& addr);

private:
    NetworkUtils() = default;
};

} // namespace PortScanner