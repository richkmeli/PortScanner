#pragma once

#include "Common.h"

namespace PortScanner {

class ConfigManager {
public:
    // Load configuration from file
    static ScanConfig load_from_file(const std::string& filename);
    
    // Save configuration to file
    static bool save_to_file(const ScanConfig& config, const std::string& filename);
    
    // Create default configuration
    static ScanConfig create_default_config();
    
    // Merge command line args with config file
    static ScanConfig merge_configs(const ScanConfig& file_config, 
                                   const ScanConfig& cli_config);
    
    // Helper methods
    static std::vector<Port> parse_port_string(const std::string& port_str);
    static ScanType string_to_scan_type(const std::string& type_str);
    static std::string scan_type_to_string(ScanType type);
    static IPVersion string_to_ip_version(const std::string& version_str);
    static std::string ip_version_to_string(IPVersion version);

private:
    // JSON support (simplified)
    static ScanConfig load_json_config(const std::string& filename);
    static bool save_json_config(const ScanConfig& config, const std::string& filename);
    
    // XML support (basic)
    static ScanConfig load_xml_config(const std::string& filename);
    static bool save_xml_config(const ScanConfig& config, const std::string& filename);
};

} // namespace PortScanner