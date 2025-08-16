# =============================================================================
# Server Component CMakeLists.txt
# =============================================================================

# Server project name
set(SERVER_TARGET_NAME mini-server)

# =============================================================================
# Server Target
# =============================================================================

# Collect server source files
file(GLOB_RECURSE SERVER_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE SERVER_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

# Group files by folder structure for IDEs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "server" FILES ${SERVER_HEADER_FILES} ${SERVER_SOURCE_FILES})

# Create server executable
add_executable(${SERVER_TARGET_NAME} ${SERVER_SOURCE_FILES} ${SERVER_HEADER_FILES})

# Set include directories for server
target_include_directories(${SERVER_TARGET_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/server
    ${CMAKE_CURRENT_SOURCE_DIR}/source/third_party
)

# =============================================================================
# Link Libraries
# =============================================================================

target_include_directories(${SERVER_TARGET_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Server libraries
target_link_libraries(${SERVER_TARGET_NAME} 
    Threads::Threads
)

# Windows specific libraries
if(WIN32)
    target_link_libraries(${SERVER_TARGET_NAME}
        ws2_32      # Winsock
        winmm       # Windows multimedia
    )
endif()

# =============================================================================
# Compiler Specific Settings
# =============================================================================

# MSVC specific settings
if(MSVC)
    target_compile_definitions(${SERVER_TARGET_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _WINSOCK_DEPRECATED_NO_WARNINGS
    )
    
    # Set runtime library
    set_property(TARGET ${SERVER_TARGET_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
    
endif()

# Install header files (by module)
install(DIRECTORY source/server/core/ DESTINATION include/core FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY source/server/net/ DESTINATION include/net FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY source/server/utils/ DESTINATION include/utils FILES_MATCHING PATTERN "*.hpp")

# =============================================================================
# Package Configuration
# =============================================================================

set(CPACK_PACKAGE_NAME ${SERVER_TARGET_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "Mini Server Team")

include(CPack)

# =============================================================================
# Build Information
# =============================================================================

message(STATUS "=== ${SERVER_TARGET_NAME} v${PROJECT_VERSION} ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Target platform: ${CMAKE_SYSTEM_NAME}")
if(WIN32)
    message(STATUS "Windows libraries: ws2_32, winmm")
endif()
message(STATUS "Server source files:")
foreach(src ${SERVER_SOURCE_FILES})
    get_filename_component(_src_dir ${src} DIRECTORY)
    get_filename_component(_src_name ${src} NAME)
    message(STATUS "  ${_src_dir}/${_src_name}")
endforeach()
message(STATUS "Header files count: ${SERVER_HEADER_FILES}")
message(STATUS "===========================================")

# =============================================================================
# Custom Targets
# =============================================================================

# Clean all target
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMENT "Cleaning all build files"
)

# Code formatting target (if clang-format is available)
find_program(CLANG_FORMAT_EXE clang-format)
if(CLANG_FORMAT_EXE)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i -style=Google ${SERVER_SOURCE_FILES} ${CLIENT_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting source code with clang-format"
    )
endif()

# Documentation target (if Doxygen is available)
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
    )
endif()
