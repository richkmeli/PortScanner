# PortScanner v2.1.0 - Advanced Edition

A modern, high-performance port scanner written in C++17 with advanced features including IPv6 support, async I/O, service detection, and configuration management.

## Features

### **Advanced Scanning Capabilities**
- **IPv6 Support**: Full IPv6 scanning with automatic detection
- **High-Performance Async I/O**: Epoll-based scanning for maximum speed
- **Advanced Service Detection**: Nmap-style service identification
- **Banner Grabbing**: Protocol-specific banner collection
- **Multiple Scan Types**: TCP Connect, SYN, UDP, ACK, FIN scans

### **Configuration Management**
- **JSON/XML Config Files**: Complex scan configurations
- **Command-line Integration**: CLI args override config files
- **Multiple Output Formats**: TXT, JSON, XML with detailed reporting
- **Performance Profiles**: Optimized settings for different scenarios

### **Enhanced User Experience**
- **Real-time Progress**: Visual progress bars with statistics
- **Intelligent Defaults**: Smart port selection and timing
- **Signal Handling**: Graceful interruption and cleanup
- **Comprehensive Help**: Detailed usage examples and tips

## Requirements

- **Operating System**: Linux/Unix-based systems
- **Compiler**: GCC 7+ or Clang 5+ with C++17 support
- **Build System**: CMake 3.16+ (recommended) or Make
- **Privileges**: Root privileges required for SYN/ACK/FIN scanning
- **Memory**: ~1MB base + (threads × 8KB)

## Installation

### Quick Install
```bash
git clone <repository-url>
cd PortScanner
./build.sh
```

### Advanced Build Options
```bash
# High-performance build
./build.sh cmake

# Debug build
./build.sh debug

# Manual compilation
./build.sh manual

# Install system-wide
sudo ./build.sh install
```

## Usage

### Basic Scanning
```bash
# Quick scan with defaults
./PortScanner google.com

# Scan specific ports
./PortScanner -p 80,443,8080 example.com

# Verbose output with service detection
./PortScanner -v -p 1-1000 192.168.1.1
```

### Advanced Scanning
```bash
# High-performance full port scan
./PortScanner -P -j 1000 -p 1-65535 target.com

# Stealth SYN scan (requires root)
sudo ./PortScanner -s syn -p 22,80,443 target.com

# IPv6 scanning
./PortScanner -6 -p 80,443 2001:db8::1

# UDP service discovery
./PortScanner -s udp -p 53,67,123,161 server.local
```

### Configuration Files
```bash
# Use JSON configuration
./PortScanner -c examples/high_performance.json

# Save results in XML format
./PortScanner -o results.xml -f xml target.com

# Load config and override specific options
./PortScanner -c config.json -p 1-1000 -j 500
```

## Command Line Options

| Option | Long Form | Description | Default |
|--------|-----------|-------------|---------|
| `-h` | `--help` | Show help message | - |
| `-V` | `--version` | Show version information | - |
| `-v` | `--verbose` | Enable verbose output | false |
| `-t` | `--target` | Target IP address or hostname | 127.0.0.1 |
| `-p` | `--ports` | Port specification | Common ports |
| `-T` | `--timeout` | Timeout in milliseconds | 3000 |
| `-j` | `--threads` | Number of threads (max: 2000) | 100 |
| `-s` | `--scan-type` | Scan type: tcp, syn, udp, ack, fin | tcp |
| `-6` | `--ipv6` | Force IPv6 scanning | auto-detect |
| `-c` | `--config` | Configuration file (JSON/XML) | - |
| `-o` | `--output` | Output file path | auto-generated |
| `-f` | `--format` | Output format: txt, json, xml | txt |
| `-S` | `--no-service-detection` | Disable service detection | enabled |
| `-B` | `--no-banner-grab` | Disable banner grabbing | enabled |
| `-P` | `--performance` | Enable high-performance mode | false |

## Scan Types Comparison

| Type | Speed | Stealth | Accuracy | Privileges | Use Case |
|------|-------|---------|----------|------------|----------|
| **TCP Connect** | Medium | Low | High | User | General scanning |
| **TCP SYN** | Fast | High | High | Root | Stealth scanning |
| **UDP** | Slow | High | Medium | User | Service discovery |
| **TCP ACK** | Fast | High | Medium | Root | Firewall testing |
| **TCP FIN** | Fast | High | Medium | Root | Stealth scanning |

## Performance Optimization

### Thread Configuration
```bash
# Conservative (low-end systems)
./PortScanner -j 50 target.com

# Balanced (modern systems)
./PortScanner -j 200 target.com

# Aggressive (high-performance)
./PortScanner -P -j 1000 target.com
```

### Timeout Tuning
```bash
# Local network (fast)
./PortScanner -T 1000 192.168.1.1

# Internet hosts (balanced)
./PortScanner -T 3000 example.com

# Slow connections (patient)
./PortScanner -T 10000 slow-server.com
```

### High-Performance Mode
When enabled with `-P`, the scanner uses:
- Async I/O with epoll for maximum concurrency
- Optimized socket options and batching
- Intelligent connection management
- Automatic performance scaling

## Configuration Files

### JSON Example
```json
{
  "target": "example.com",
  "ports": [80, 443, 8080, 8443],
  "scan_type": "tcp",
  "timeout": 3000,
  "threads": 200,
  "service_detection": true,
  "banner_grabbing": true,
  "output_format": "json"
}
```

### XML Example
```xml
<?xml version="1.0" encoding="UTF-8"?>
<scan_config>
  <target>192.168.1.0/24</target>
  <ports>22,80,443,3389</ports>
  <scan_type>syn</scan_type>
  <threads>500</threads>
  <verbose>true</verbose>
</scan_config>
```

## Service Detection

The scanner includes advanced service detection capabilities:

### Supported Protocols
- **HTTP/HTTPS**: Server identification, version detection
- **SSH**: Version and implementation detection
- **FTP**: Server software identification
- **SMTP/POP3/IMAP**: Mail server detection
- **DNS**: Service identification
- **Database**: MySQL, PostgreSQL, Redis, MongoDB detection

### Banner Grabbing
- Protocol-specific banner collection
- HTTP header analysis
- SSL/TLS certificate information
- Custom service fingerprinting

## Output Formats

### Console Output
- **Summary**: Quick overview with open ports
- **Detailed**: Complete scan results with timing
- **Progress**: Real-time scanning progress

### File Formats
- **TXT**: Human-readable detailed reports
- **JSON**: Machine-readable structured data
- **XML**: Structured markup for integration

## Examples

### Network Discovery
```bash
# Scan common ports on local network
./PortScanner -p 22,80,443,3389 192.168.1.0/24
```

### Web Application Testing
```bash
# Comprehensive web server analysis
./PortScanner -v -p 80,443,8000-8090,9000-9090 webserver.com
```

### Security Assessment
```bash
# Full port scan with service detection
sudo ./PortScanner -s syn -P -j 1000 -p 1-65535 -v target.com
```

### Service Discovery
```bash
# UDP service enumeration
./PortScanner -s udp -p 53,67,68,123,161,162,514 -T 5000 server.local
```

## Troubleshooting

### Common Issues

1. **Permission Denied for Advanced Scans**
   ```bash
   sudo ./PortScanner -s syn  # Run with root privileges
   ```

2. **High Memory Usage**
   ```bash
   ./PortScanner -j 100  # Reduce thread count
   ```

3. **Slow Scanning**
   ```bash
   ./PortScanner -P -T 1000  # Enable performance mode, reduce timeout
   ```

4. **Network Timeouts**
   ```bash
   ./PortScanner -T 10000  # Increase timeout for slow networks
   ```

### Performance Tips

- Use high-performance mode (`-P`) for large scans
- Adjust thread count based on system capabilities
- Configure timeouts based on network conditions
- Use configuration files for complex setups
- Monitor system resources during intensive scans

## Security Considerations

- **Authorized Testing Only**: Only scan systems you own or have permission to test
- **Rate Limiting**: Respect network policies and rate limits
- **Firewall Detection**: Some networks may detect and block scanning
- **Legal Compliance**: Ensure compliance with local laws and regulations
- **Responsible Disclosure**: Report vulnerabilities through proper channels

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

Copyright (c) 2024. All rights reserved.

---

**PortScanner v2.1.0** - Advanced C++ port scanner with enterprise-level features 🚀