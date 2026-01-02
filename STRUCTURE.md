# PortScanner v2.1.0 - Project Structure

## Directory Layout

```
PortScanner/
├── CMakeLists.txt          # CMake build configuration
├── Makefile               # Alternative build system
├── build.sh              # Automated build script
├── README.md             # Project documentation
├── .gitignore           # Git ignore rules
├── LICENSE              # License file
│
├── include/             # Header files
│   ├── Common.h         # Common types and constants
│   ├── ArgumentsManager.h   # Command-line argument handling
│   ├── PortScanner.h    # Main scanner class
│   ├── NetworkUtils.h   # Network utility functions
│   ├── ScanResults.h    # Result management
│   ├── ServiceDetector.h # Service detection
│   ├── AsyncScanner.h   # High-performance scanning
│   └── ConfigManager.h  # Configuration management
│
├── src/                 # Source files
│   ├── main.cpp         # Application entry point
│   ├── ArgumentsManager.cpp # Argument parsing implementation
│   ├── PortScanner.cpp  # Scanner implementation
│   ├── NetworkUtils.cpp # Network utilities implementation
│   ├── ScanResults.cpp  # Results management implementation
│   ├── ServiceDetector.cpp # Service detection implementation
│   ├── AsyncScanner.cpp # Async scanning implementation
│   └── ConfigManager.cpp # Configuration implementation
│
├── examples/            # Configuration examples
│   ├── default_config.json
│   ├── high_performance.json
│   └── web_scan.xml
│
├── build/               # Build artifacts (created during build)
└── tests/               # Test files
```

## Architecture Overview

### Modular Design
- **Separated Concerns**: Each component has a specific responsibility
- **Namespace Organization**: All code under `PortScanner` namespace
- **RAII Principles**: Automatic resource management
- **Exception Safety**: Comprehensive error handling

### Security Features
- **Input Validation**: All user inputs are validated
- **Buffer Overflow Protection**: Safe string handling
- **Memory Management**: No memory leaks with proper RAII
- **Privilege Separation**: Clear distinction between privileged operations

### Performance Features
- **Multi-threading**: Configurable thread pool for parallel scanning
- **Async I/O**: Epoll-based high-performance scanning
- **Efficient Algorithms**: Optimized scanning algorithms
- **Resource Management**: Minimal memory footprint
- **Scalability**: Handles large port ranges efficiently

### Code Quality
- **C++17 Standards**: Modern C++ features and best practices
- **Type Safety**: Strong typing with type aliases
- **Const Correctness**: Proper const usage throughout
- **Documentation**: Comprehensive inline documentation

## Build Systems

### 1. CMake (Recommended)
- Cross-platform build system
- Automatic dependency detection
- Optimized compilation flags
- Easy integration with IDEs

### 2. Makefile
- Traditional Unix build system
- No external dependencies
- Simple and reliable
- Supports multiple targets

### 3. Manual Compilation
- Direct g++ compilation
- Minimal requirements
- Quick for development
- Fallback option

## Testing Strategy

The build script includes basic functionality tests:
- Command-line option validation
- Help and version display
- Basic connectivity tests

## Development Guidelines

### Code Style
- Follow Google C++ Style Guide
- Use clang-format for formatting
- Maintain const correctness
- Prefer RAII over manual resource management

### Error Handling
- Use exceptions for error conditions
- Provide meaningful error messages
- Validate all inputs
- Handle system call failures gracefully

### Documentation
- Document all public interfaces
- Provide usage examples
- Maintain README accuracy
- Include performance notes