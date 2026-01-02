cmake_minimum_required(VERSION 3.16)
project(PortScanner VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/ArgumentsManager.cpp
    src/PortScanner.cpp
    src/NetworkUtils.cpp
    src/ScanResults.cpp
    src/ServiceDetector.cpp
    src/AsyncScanner.cpp
    src/ConfigManager.cpp
)

# Headers
set(HEADERS
    include/ArgumentsManager.h
    include/PortScanner.h
    include/NetworkUtils.h
    include/ScanResults.h
    include/Common.h
    include/ServiceDetector.h
    include/AsyncScanner.h
    include/ConfigManager.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} pthread)

# Install
install(TARGETS ${PROJECT_NAME} DESTINATION bin)