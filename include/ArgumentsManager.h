#pragma once

#include "Common.h"
#include <optional>
#include <stdexcept>

namespace PortScanner {

class ArgumentsManager {
public:
    explicit ArgumentsManager(int argc, char* argv[]);
    
    const ScanConfig& get_config() const noexcept { return config_; }
    bool should_exit() const noexcept { return should_exit_; }
    
    static void print_help();
    static void print_version();

private:
    ScanConfig config_;
    bool should_exit_ = false;
    
    void parse_arguments(int argc, char* argv[]);
    void validate_config();
    std::vector<Port> parse_port_range(const std::string& port_str);
};

class ArgumentError : public std::runtime_error {
public:
    explicit ArgumentError(const std::string& message) 
        : std::runtime_error("Argument error: " + message) {}
};

} // namespace PortScanner