cmake_minimum_required(VERSION 3.15)

project(MiniServerProject 
    VERSION 1.0.0 
    DESCRIPTION "A lightweight HTTP server with test client"
    LANGUAGES CXX
)

# =============================================================================
# Global Project Configuration
# =============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Global compiler options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Global Debug/Release configuration
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/ZI /Od /DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# Set global output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Per-configuration output directories
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin/${OUTPUTCONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib/${OUTPUTCONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib/${OUTPUTCONFIG})
endforeach()

# =============================================================================
# Dependencies
# =============================================================================

find_package(Threads REQUIRED)

# =============================================================================
# Add Subdirectories
# =============================================================================

# Add server subdirectory
add_subdirectory(source/server)

# Add client subdirectory
add_subdirectory(source/client)

# =============================================================================
# Summary
# =============================================================================

message(STATUS "")
message(STATUS "=== MiniServerProject Configuration Summary ===")
message(STATUS "Project Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "")
message(STATUS "Components:")
message(STATUS "  - Server: source/server")
message(STATUS "  - Client: source/client")
message(STATUS "")
message(STATUS "Output Directories:")
message(STATUS "  - Runtime: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "  - Library: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message(STATUS "  - Archive: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
message(STATUS "===============================================")
message(STATUS "")
