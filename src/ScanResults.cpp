#include "ScanResults.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace PortScanner {

void ScanResults::add_result(const ScanResult& result) {
    results_.push_back(result);
}

void ScanResults::add_result(Port port, PortStatus status, Duration response_time, const std::string& service) {
    ServiceInfo service_info;
    service_info.name = service;
    results_.emplace_back(ScanResult{port, status, response_time, service_info, ""});
}

std::size_t ScanResults::open_count() const noexcept {
    return std::count_if(results_.begin(), results_.end(),
                        [](const ScanResult& r) { return r.status == PortStatus::OPEN; });
}

std::size_t ScanResults::closed_count() const noexcept {
    return std::count_if(results_.begin(), results_.end(),
                        [](const ScanResult& r) { return r.status == PortStatus::CLOSED; });
}

std::size_t ScanResults::filtered_count() const noexcept {
    return std::count_if(results_.begin(), results_.end(),
                        [](const ScanResult& r) { return r.status == PortStatus::FILTERED; });
}

std::vector<ScanResult> ScanResults::get_open_ports() const {
    std::vector<ScanResult> open_ports;
    std::copy_if(results_.begin(), results_.end(), std::back_inserter(open_ports),
                [](const ScanResult& r) { return r.status == PortStatus::OPEN; });
    return open_ports;
}

void ScanResults::print_summary(std::ostream& os) const {
    os << "=== SCAN SUMMARY ===\n";
    os << "Total ports scanned: " << total_count() << "\n";
    os << "Open ports: " << open_count() << "\n";
    os << "Closed ports: " << closed_count() << "\n";
    os << "Filtered ports: " << filtered_count() << "\n\n";
    
    auto open_ports = get_open_ports();
    if (!open_ports.empty()) {
        os << "=== OPEN PORTS ===\n";
        os << std::left << std::setw(8) << "PORT" 
           << std::setw(12) << "STATE" 
           << std::setw(15) << "SERVICE"
           << std::setw(12) << "RESPONSE" << "\n";
        os << std::string(47, '-') << "\n";
        
        for (const auto& result : open_ports) {
            os << std::left << std::setw(8) << result.port
               << std::setw(12) << status_to_string(result.status)
               << std::setw(15) << (result.service.name.empty() ? "unknown" : result.service.name)
               << std::setw(12) << (std::to_string(result.response_time.count()) + "ms") << "\n";
        }
    }
}

void ScanResults::print_detailed(std::ostream& os) const {
    os << "=== DETAILED SCAN RESULTS ===\n";
    os << std::left << std::setw(8) << "PORT" 
       << std::setw(12) << "STATE" 
       << std::setw(15) << "SERVICE"
       << std::setw(12) << "RESPONSE" << "\n";
    os << std::string(47, '-') << "\n";
    
    // Sort results by port number
    auto sorted_results = results_;
    std::sort(sorted_results.begin(), sorted_results.end(),
             [](const ScanResult& a, const ScanResult& b) { return a.port < b.port; });
    
    for (const auto& result : sorted_results) {
        os << std::left << std::setw(8) << result.port
           << std::setw(12) << status_to_string(result.status)
           << std::setw(15) << (result.service.name.empty() ? "unknown" : result.service.name)
           << std::setw(12) << (std::to_string(result.response_time.count()) + "ms") << "\n";
    }
    
    os << "\n";
    print_summary(os);
}

bool ScanResults::save_to_file(const std::string& filename, const std::string& format) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        if (format == "json") {
            save_as_json(file);
        } else if (format == "xml") {
            save_as_xml(file);
        } else {
            save_as_txt(file);
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void ScanResults::save_as_txt(std::ofstream& file) const {
    file << "PortScanner Results\n";
    file << "==================\n\n";
    
    // Redirect detailed output to file
    print_detailed(file);
}

void ScanResults::save_as_json(std::ofstream& file) const {
    file << "{\n";
    file << "  \"scan_results\": {\n";
    file << "    \"total_ports\": " << total_count() << ",\n";
    file << "    \"open_ports\": " << open_count() << ",\n";
    file << "    \"closed_ports\": " << closed_count() << ",\n";
    file << "    \"filtered_ports\": " << filtered_count() << ",\n";
    file << "    \"ports\": [\n";
    
    for (std::size_t i = 0; i < results_.size(); ++i) {
        const auto& result = results_[i];
        file << "      {\n";
        file << "        \"port\": " << result.port << ",\n";
        file << "        \"status\": \"" << status_to_string(result.status) << "\",\n";
        file << "        \"service\": \"" << result.service.name << "\",\n";
        file << "        \"response_time_ms\": " << result.response_time.count() << "\n";
        file << "      }";
        if (i < results_.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "    ]\n";
    file << "  }\n";
    file << "}\n";
}

void ScanResults::save_as_xml(std::ofstream& file) const {
    file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    file << "<scan_results>\n";
    file << "  <summary>\n";
    file << "    <total_ports>" << total_count() << "</total_ports>\n";
    file << "    <open_ports>" << open_count() << "</open_ports>\n";
    file << "    <closed_ports>" << closed_count() << "</closed_ports>\n";
    file << "    <filtered_ports>" << filtered_count() << "</filtered_ports>\n";
    file << "  </summary>\n";
    file << "  <ports>\n";
    
    for (const auto& result : results_) {
        file << "    <port>\n";
        file << "      <number>" << result.port << "</number>\n";
        file << "      <status>" << status_to_string(result.status) << "</status>\n";
        file << "      <service>" << result.service.name << "</service>\n";
        file << "      <response_time_ms>" << result.response_time.count() << "</response_time_ms>\n";
        file << "    </port>\n";
    }
    
    file << "  </ports>\n";
    file << "</scan_results>\n";
}

std::string ScanResults::status_to_string(PortStatus status) const {
    switch (status) {
        case PortStatus::OPEN: return "open";
        case PortStatus::CLOSED: return "closed";
        case PortStatus::FILTERED: return "filtered";
        case PortStatus::UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

} // namespace PortScanner