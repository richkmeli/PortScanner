#include "ConfigManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace PortScanner {

ScanConfig ConfigManager::load_from_file(const std::string& filename) {
    std::string extension = filename.substr(filename.find_last_of('.') + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == "json") {
        return load_json_config(filename);
    } else if (extension == "xml") {
        return load_xml_config(filename);
    } else {
        throw std::runtime_error("Unsupported config file format: " + extension);
    }
}

bool ConfigManager::save_to_file(const ScanConfig& config, const std::string& filename) {
    std::string extension = filename.substr(filename.find_last_of('.') + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == "json") {
        return save_json_config(config, filename);
    } else if (extension == "xml") {
        return save_xml_config(config, filename);
    }
    
    return false;
}

ScanConfig ConfigManager::create_default_config() {
    ScanConfig config;
    config.target = "127.0.0.1";
    config.ports = {21, 22, 23, 25, 53, 80, 110, 111, 135, 139, 143, 443, 993, 995, 1723, 3306, 3389, 5432, 5900, 8080};
    config.scan_type = ScanType::TCP_CONNECT;
    config.ip_version = IPVersion::AUTO;
    config.timeout = DEFAULT_TIMEOUT;
    config.thread_count = DEFAULT_THREAD_COUNT;
    config.verbose = false;
    config.service_detection = true;
    config.banner_grabbing = true;
    config.output_format = "txt";
    
    return config;
}

ScanConfig ConfigManager::merge_configs(const ScanConfig& file_config, const ScanConfig& cli_config) {
    ScanConfig merged = file_config;
    
    // CLI arguments override file config
    if (!cli_config.target.empty() && cli_config.target != "127.0.0.1") {
        merged.target = cli_config.target;
    }
    
    if (!cli_config.ports.empty()) {
        merged.ports = cli_config.ports;
    }
    
    if (cli_config.scan_type != ScanType::TCP_CONNECT) {
        merged.scan_type = cli_config.scan_type;
    }
    
    if (cli_config.timeout != DEFAULT_TIMEOUT) {
        merged.timeout = cli_config.timeout;
    }
    
    if (cli_config.thread_count != DEFAULT_THREAD_COUNT) {
        merged.thread_count = cli_config.thread_count;
    }
    
    merged.verbose = cli_config.verbose || file_config.verbose;
    
    if (!cli_config.output_file.empty()) {
        merged.output_file = cli_config.output_file;
    }
    
    if (cli_config.output_format != "txt") {
        merged.output_format = cli_config.output_format;
    }
    
    return merged;
}

// Simplified JSON implementation without external dependencies
ScanConfig ConfigManager::load_json_config(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + filename);
    }
    
    ScanConfig config = create_default_config();
    std::string line;
    
    // Simple JSON parsing (basic implementation)
    while (std::getline(file, line)) {
        // Remove whitespace
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        
        if (line.find("\"target\":") != std::string::npos) {
            std::size_t start = line.find('"', line.find(':')) + 1;
            std::size_t end = line.find('"', start);
            if (start != std::string::npos && end != std::string::npos) {
                config.target = line.substr(start, end - start);
            }
        } else if (line.find("\"ports\":") != std::string::npos) {
            std::size_t start = line.find('[') + 1;
            std::size_t end = line.find(']');
            if (start != std::string::npos && end != std::string::npos) {
                std::string ports_str = line.substr(start, end - start);
                config.ports = parse_port_string(ports_str);
            }
        } else if (line.find("\"scan_type\":") != std::string::npos) {
            std::size_t start = line.find('"', line.find(':')) + 1;
            std::size_t end = line.find('"', start);
            if (start != std::string::npos && end != std::string::npos) {
                config.scan_type = string_to_scan_type(line.substr(start, end - start));
            }
        } else if (line.find("\"timeout\":") != std::string::npos) {
            std::size_t start = line.find(':') + 1;
            std::size_t end = line.find(',', start);
            if (end == std::string::npos) end = line.length();
            
            std::string timeout_str = line.substr(start, end - start);
            timeout_str.erase(std::remove_if(timeout_str.begin(), timeout_str.end(), 
                             [](char c) { return !std::isdigit(c); }), timeout_str.end());
            
            if (!timeout_str.empty()) {
                config.timeout = Duration{std::stoi(timeout_str)};
            }
        } else if (line.find("\"threads\":") != std::string::npos) {
            std::size_t start = line.find(':') + 1;
            std::size_t end = line.find(',', start);
            if (end == std::string::npos) end = line.length();
            
            std::string threads_str = line.substr(start, end - start);
            threads_str.erase(std::remove_if(threads_str.begin(), threads_str.end(), 
                             [](char c) { return !std::isdigit(c); }), threads_str.end());
            
            if (!threads_str.empty()) {
                config.thread_count = std::stoul(threads_str);
            }
        }
    }
    
    return config;
}

bool ConfigManager::save_json_config(const ScanConfig& config, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << "{\n";
    file << "  \"target\": \"" << config.target << "\",\n";
    file << "  \"ports\": [";
    
    for (std::size_t i = 0; i < config.ports.size(); ++i) {
        file << config.ports[i];
        if (i < config.ports.size() - 1) file << ", ";
    }
    
    file << "],\n";
    file << "  \"scan_type\": \"" << scan_type_to_string(config.scan_type) << "\",\n";
    file << "  \"ip_version\": \"" << ip_version_to_string(config.ip_version) << "\",\n";
    file << "  \"timeout\": " << config.timeout.count() << ",\n";
    file << "  \"threads\": " << config.thread_count << ",\n";
    file << "  \"verbose\": " << (config.verbose ? "true" : "false") << ",\n";
    file << "  \"service_detection\": " << (config.service_detection ? "true" : "false") << ",\n";
    file << "  \"banner_grabbing\": " << (config.banner_grabbing ? "true" : "false") << ",\n";
    file << "  \"output_format\": \"" << config.output_format << "\"\n";
    file << "}\n";
    
    return true;
}

ScanConfig ConfigManager::load_xml_config(const std::string& filename) {
    // Basic XML parsing - simplified implementation
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + filename);
    }
    
    ScanConfig config = create_default_config();
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    // Simple XML tag extraction
    auto extract_tag_value = [&content](const std::string& tag) -> std::string {
        std::string start_tag = "<" + tag + ">";
        std::string end_tag = "</" + tag + ">";
        
        std::size_t start = content.find(start_tag);
        if (start == std::string::npos) return "";
        
        start += start_tag.length();
        std::size_t end = content.find(end_tag, start);
        if (end == std::string::npos) return "";
        
        return content.substr(start, end - start);
    };
    
    std::string target = extract_tag_value("target");
    if (!target.empty()) config.target = target;
    
    std::string scan_type = extract_tag_value("scan_type");
    if (!scan_type.empty()) config.scan_type = string_to_scan_type(scan_type);
    
    std::string timeout = extract_tag_value("timeout");
    if (!timeout.empty()) config.timeout = Duration{std::stoi(timeout)};
    
    std::string threads = extract_tag_value("threads");
    if (!threads.empty()) config.thread_count = std::stoul(threads);
    
    return config;
}

bool ConfigManager::save_xml_config(const ScanConfig& config, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    file << "<scan_config>\n";
    file << "  <target>" << config.target << "</target>\n";
    file << "  <ports>";
    for (std::size_t i = 0; i < config.ports.size(); ++i) {
        file << config.ports[i];
        if (i < config.ports.size() - 1) file << ",";
    }
    file << "</ports>\n";
    file << "  <scan_type>" << scan_type_to_string(config.scan_type) << "</scan_type>\n";
    file << "  <ip_version>" << ip_version_to_string(config.ip_version) << "</ip_version>\n";
    file << "  <timeout>" << config.timeout.count() << "</timeout>\n";
    file << "  <threads>" << config.thread_count << "</threads>\n";
    file << "  <verbose>" << (config.verbose ? "true" : "false") << "</verbose>\n";
    file << "  <service_detection>" << (config.service_detection ? "true" : "false") << "</service_detection>\n";
    file << "  <banner_grabbing>" << (config.banner_grabbing ? "true" : "false") << "</banner_grabbing>\n";
    file << "  <output_format>" << config.output_format << "</output_format>\n";
    file << "</scan_config>\n";
    
    return true;
}

std::vector<Port> ConfigManager::parse_port_string(const std::string& port_str) {
    std::vector<Port> ports;
    std::istringstream iss(port_str);
    std::string token;
    
    while (std::getline(iss, token, ',')) {
        token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());
        if (!token.empty()) {
            ports.push_back(static_cast<Port>(std::stoi(token)));
        }
    }
    
    return ports;
}

ScanType ConfigManager::string_to_scan_type(const std::string& type_str) {
    std::string lower_type = type_str;
    std::transform(lower_type.begin(), lower_type.end(), lower_type.begin(), ::tolower);
    
    if (lower_type == "tcp" || lower_type == "connect") return ScanType::TCP_CONNECT;
    if (lower_type == "syn") return ScanType::TCP_SYN;
    if (lower_type == "udp") return ScanType::UDP;
    if (lower_type == "ack") return ScanType::TCP_ACK;
    if (lower_type == "fin") return ScanType::TCP_FIN;
    
    return ScanType::TCP_CONNECT;
}

std::string ConfigManager::scan_type_to_string(ScanType type) {
    switch (type) {
        case ScanType::TCP_CONNECT: return "tcp";
        case ScanType::TCP_SYN: return "syn";
        case ScanType::UDP: return "udp";
        case ScanType::TCP_ACK: return "ack";
        case ScanType::TCP_FIN: return "fin";
        default: return "tcp";
    }
}

IPVersion ConfigManager::string_to_ip_version(const std::string& version_str) {
    std::string lower_version = version_str;
    std::transform(lower_version.begin(), lower_version.end(), lower_version.begin(), ::tolower);
    
    if (lower_version == "ipv4" || lower_version == "4") return IPVersion::IPv4;
    if (lower_version == "ipv6" || lower_version == "6") return IPVersion::IPv6;
    
    return IPVersion::AUTO;
}

std::string ConfigManager::ip_version_to_string(IPVersion version) {
    switch (version) {
        case IPVersion::IPv4: return "ipv4";
        case IPVersion::IPv6: return "ipv6";
        case IPVersion::AUTO: return "auto";
        default: return "auto";
    }
}

} // namespace PortScanner