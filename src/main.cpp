#include "ArgumentsManager.h"
#include "PortScanner.h"
#include "ConfigManager.h"
#include <iostream>
#include <iomanip>
#include <csignal>
#include <atomic>

namespace {
    std::atomic<bool> interrupted{false};
    
    void signal_handler(int signal) {
        if (signal == SIGINT || signal == SIGTERM) {
            interrupted.store(true);
            std::cout << "\n\nScan interrupted by user.\n";
        }
    }
    
    void setup_signal_handlers() {
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
    }
    
    void print_progress_bar(std::size_t completed, std::size_t total) {
        const int bar_width = 50;
        const double progress = static_cast<double>(completed) / total;
        const int pos = static_cast<int>(bar_width * progress);
        
        std::cout << "\r[";
        for (int i = 0; i < bar_width; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << std::fixed << std::setprecision(1) 
                  << (progress * 100.0) << "% (" << completed << "/" << total << ")";
        std::cout.flush();
        
        if (completed == total) {
            std::cout << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    try {
        setup_signal_handlers();
        
        PortScanner::ArgumentsManager args_manager(argc, argv);
        
        if (args_manager.should_exit()) {
            return 0;
        }
        
        auto config = args_manager.get_config();
        
        // Load configuration file if specified
        if (!config.config_file.empty()) {
            try {
                auto file_config = PortScanner::ConfigManager::load_from_file(config.config_file);
                config = PortScanner::ConfigManager::merge_configs(file_config, config);
            } catch (const std::exception& e) {
                std::cerr << "Warning: Failed to load config file: " << e.what() << "\n";
            }
        }
        
        std::cout << "PortScanner v2.1.0 - Advanced Edition\n";
        std::cout << "Target: " << config.target << "\n";
        std::cout << "Ports: " << config.ports.size() << " ports to scan\n";
        std::cout << "Scan Type: " << PortScanner::ConfigManager::scan_type_to_string(config.scan_type) << "\n";
        std::cout << "Threads: " << config.thread_count << "\n";
        std::cout << "Timeout: " << config.timeout.count() << "ms\n";
        std::cout << "Service Detection: " << (config.service_detection ? "enabled" : "disabled") << "\n";
        std::cout << "Banner Grabbing: " << (config.banner_grabbing ? "enabled" : "disabled") << "\n\n";
        
        // Create scanner with enhanced configuration
        PortScanner::PortScanner scanner(config);
        
        // Enable high-performance mode for large scans
        if (config.ports.size() > 1000 || config.thread_count > 200) {
            scanner.set_performance_mode(true);
            std::cout << "High-performance async mode enabled\n\n";
        }
        
        auto progress_callback = [](std::size_t completed, std::size_t total) {
            if (!interrupted.load()) {
                print_progress_bar(completed, total);
            }
        };
        
        // Use async scanning for better performance
        auto future_results = scanner.scan_ports_async(progress_callback);
        
        // Wait for results or interruption
        auto results = future_results.get();
        
        if (!interrupted.load()) {
            std::cout << "\nScan completed!\n\n";
            
            if (config.verbose) {
                results.print_detailed();
            } else {
                results.print_summary();
            }
            
            // Save results if there are open ports or output file specified
            if (results.open_count() > 0 || !config.output_file.empty()) {
                std::string filename = config.output_file;
                if (filename.empty()) {
                    filename = "scan_results_" + config.target + "." + config.output_format;
                }
                
                if (results.save_to_file(filename, config.output_format)) {
                    std::cout << "\nResults saved to: " << filename << "\n";
                }
            }
            
            // Save configuration for future use
            if (!config.config_file.empty()) {
                PortScanner::ConfigManager::save_to_file(config, config.config_file);
            }
        } else {
            scanner.cancel_scan();
        }
        
        return 0;
        
    } catch (const PortScanner::ArgumentError& e) {
        std::cerr << "Error: " << e.what() << "\n\n";
        PortScanner::ArgumentsManager::print_help();
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}