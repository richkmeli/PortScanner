#pragma once

#include "Common.h"
#include <iostream>
#include <fstream>

namespace PortScanner {

class ScanResults {
public:
    ScanResults() = default;
    
    void add_result(const ScanResult& result);
    void add_result(Port port, PortStatus status, Duration response_time = Duration{0}, 
                   const std::string& service = "");
    
    std::size_t total_count() const noexcept { return results_.size(); }
    std::size_t open_count() const noexcept;
    std::size_t closed_count() const noexcept;
    std::size_t filtered_count() const noexcept;
    
    const std::vector<ScanResult>& get_results() const noexcept { return results_; }
    std::vector<ScanResult> get_open_ports() const;
    
    void print_summary(std::ostream& os = std::cout) const;
    void print_detailed(std::ostream& os = std::cout) const;
    
    bool save_to_file(const std::string& filename, const std::string& format = "txt") const;
    
    void clear() { results_.clear(); }

private:
    std::vector<ScanResult> results_;
    
    void save_as_txt(std::ofstream& file) const;
    void save_as_json(std::ofstream& file) const;
    void save_as_xml(std::ofstream& file) const;
    
    std::string status_to_string(PortStatus status) const;
};

} // namespace PortScanner