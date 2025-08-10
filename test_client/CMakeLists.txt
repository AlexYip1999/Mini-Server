cmake_minimum_required(VERSION 3.15)

project(MiniServerTestClient VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(test_client test_client.cpp)

# Link libraries for Windows
if(WIN32)
    target_link_libraries(test_client ws2_32)
endif()

# Set output directory
set_target_properties(test_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Copy to main build directory for convenience
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(test_client PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
    )
endif()
