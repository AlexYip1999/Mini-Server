# =============================================================================
# Client Component CMakeLists.txt
# =============================================================================

# Client project configuration
set(CLIENT_TARGET_NAME test-client)

# =============================================================================
# Client Source Files
# =============================================================================

# Collect client source files
file(GLOB_RECURSE CLIENT_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE CLIENT_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

# Group files for IDEs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "client" FILES ${CLIENT_SOURCE_FILES} ${CLIENT_HEADER_FILES})

# Create client executable
add_executable(${CLIENT_TARGET_NAME} ${CLIENT_SOURCE_FILES} ${CLIENT_HEADER_FILES})

# Set include directories for client
target_include_directories(${CLIENT_TARGET_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/source/third_party
)

# =============================================================================
# Client Libraries
# =============================================================================

# Client libraries
target_link_libraries(${CLIENT_TARGET_NAME} 
    Threads::Threads
)

# Windows specific libraries
if(WIN32)
    target_link_libraries(${CLIENT_TARGET_NAME}
        ws2_32      # Winsock for client
    )
endif()

# =============================================================================
# Compiler Specific Settings
# =============================================================================

# MSVC specific settings
if(MSVC)
    target_compile_definitions(${CLIENT_TARGET_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _WINSOCK_DEPRECATED_NO_WARNINGS
    )
    
    # Set runtime library
    set_property(TARGET ${CLIENT_TARGET_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif()

# =============================================================================
# Build Information
# =============================================================================

message(STATUS "=== Client Component Configuration ===")
message(STATUS "Target name: ${CLIENT_TARGET_NAME}")
message(STATUS "Client source files found: ${CLIENT_SOURCE_FILES}")
message(STATUS "Client header files found: ${CLIENT_HEADER_FILES}")
message(STATUS "=====================================")
