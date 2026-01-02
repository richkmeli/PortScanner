#!/bin/bash

# PortScanner Build Script
# Provides easy compilation options for different scenarios

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Project info
PROJECT_NAME="PortScanner"
VERSION="2.0.0"

# Functions
print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}  $PROJECT_NAME v$VERSION Build${NC}"
    echo -e "${BLUE}================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

check_dependencies() {
    print_info "Checking dependencies..."
    
    # Check for C++ compiler
    if ! command -v g++ &> /dev/null; then
        print_error "g++ compiler not found. Please install build-essential."
        exit 1
    fi
    
    # Check C++17 support
    if ! g++ -std=c++17 -x c++ -E - < /dev/null &> /dev/null; then
        print_error "C++17 support not available. Please update your compiler."
        exit 1
    fi
    
    print_success "Dependencies check passed"
}

build_cmake() {
    print_info "Building with CMake..."
    
    if ! command -v cmake &> /dev/null; then
        print_error "CMake not found. Falling back to Makefile build."
        build_makefile
        return
    fi
    
    mkdir -p build
    cd build
    
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    
    cd ..
    
    if [ -f "build/$PROJECT_NAME" ]; then
        cp "build/$PROJECT_NAME" .
        print_success "CMake build completed successfully"
    else
        print_error "CMake build failed"
        exit 1
    fi
}

build_makefile() {
    print_info "Building with Makefile..."
    
    if [ ! -f "Makefile" ]; then
        print_error "Makefile not found"
        exit 1
    fi
    
    make clean 2>/dev/null || true
    make -j$(nproc)
    
    if [ -f "$PROJECT_NAME" ]; then
        print_success "Makefile build completed successfully"
    else
        print_error "Makefile build failed"
        exit 1
    fi
}

build_manual() {
    print_info "Building manually..."
    
    # Create build directory
    mkdir -p build
    
    # Compile source files
    print_info "Compiling source files..."
    g++ -std=c++17 -O3 -Wall -Wextra -Wpedantic \
        -Iinclude \
        src/*.cpp \
        -o $PROJECT_NAME \
        -lpthread
    
    if [ -f "$PROJECT_NAME" ]; then
        print_success "Manual build completed successfully"
    else
        print_error "Manual build failed"
        exit 1
    fi
}

build_debug() {
    print_info "Building debug version..."
    
    g++ -std=c++17 -g -O0 -Wall -Wextra -Wpedantic -DDEBUG \
        -Iinclude \
        src/*.cpp \
        -o ${PROJECT_NAME}_debug \
        -lpthread
    
    if [ -f "${PROJECT_NAME}_debug" ]; then
        print_success "Debug build completed successfully"
    else
        print_error "Debug build failed"
        exit 1
    fi
}

clean_build() {
    print_info "Cleaning build artifacts..."
    
    rm -rf build/
    rm -f $PROJECT_NAME ${PROJECT_NAME}_debug
    rm -f *.o
    
    print_success "Clean completed"
}

run_tests() {
    print_info "Running basic tests..."
    
    if [ ! -f "$PROJECT_NAME" ]; then
        print_error "Executable not found. Build first."
        exit 1
    fi
    
    # Test help option
    if ./$PROJECT_NAME --help &> /dev/null; then
        print_success "Help option test passed"
    else
        print_warning "Help option test failed"
    fi
    
    # Test version option
    if ./$PROJECT_NAME --version &> /dev/null; then
        print_success "Version option test passed"
    else
        print_warning "Version option test failed"
    fi
    
    print_success "Basic tests completed"
}

install_binary() {
    if [ ! -f "$PROJECT_NAME" ]; then
        print_error "Executable not found. Build first."
        exit 1
    fi
    
    print_info "Installing to /usr/local/bin (requires sudo)..."
    sudo cp $PROJECT_NAME /usr/local/bin/
    sudo chmod +x /usr/local/bin/$PROJECT_NAME
    
    print_success "Installation completed"
}

show_usage() {
    echo "Usage: $0 [OPTION]"
    echo ""
    echo "Build options:"
    echo "  cmake     Build using CMake (recommended)"
    echo "  make      Build using Makefile"
    echo "  manual    Build manually with g++"
    echo "  debug     Build debug version"
    echo "  clean     Clean build artifacts"
    echo "  test      Run basic tests"
    echo "  install   Install to system (requires sudo)"
    echo "  help      Show this help message"
    echo ""
    echo "If no option is specified, CMake build is attempted first,"
    echo "falling back to Makefile, then manual build."
}

# Main script
print_header

case "${1:-auto}" in
    "cmake")
        check_dependencies
        build_cmake
        ;;
    "make")
        check_dependencies
        build_makefile
        ;;
    "manual")
        check_dependencies
        build_manual
        ;;
    "debug")
        check_dependencies
        build_debug
        ;;
    "clean")
        clean_build
        ;;
    "test")
        run_tests
        ;;
    "install")
        install_binary
        ;;
    "help")
        show_usage
        ;;
    "auto")
        check_dependencies
        if command -v cmake &> /dev/null; then
            build_cmake
        elif [ -f "Makefile" ]; then
            build_makefile
        else
            build_manual
        fi
        ;;
    *)
        print_error "Unknown option: $1"
        show_usage
        exit 1
        ;;
esac

print_success "Build script completed successfully!"

# Show final executable info
if [ -f "$PROJECT_NAME" ]; then
    echo ""
    print_info "Executable created: $PROJECT_NAME"
    print_info "Size: $(du -h $PROJECT_NAME | cut -f1)"
    print_info "Run with: ./$PROJECT_NAME --help"
fi