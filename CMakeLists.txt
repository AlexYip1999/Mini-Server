cmake_minimum_required(VERSION 3.15)
project(MiniServer 
    VERSION 1.0.0 
    DESCRIPTION "A lightweight HTTP server with service registration capabilities"
    LANGUAGES CXX
)

# =============================================================================
# 项目配置
# =============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 调试/发布配置
if(MSVC)
    # 替换 GCC/Clang 风格标志，使用 MSVC 等价参数，避免 D9002 警告
    set(CMAKE_CXX_FLAGS_DEBUG "/ZI /Od /DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# =============================================================================
# 依赖项
# =============================================================================

find_package(Threads REQUIRED)

# =============================================================================
# 包含目录
# =============================================================================

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# =============================================================================
# 源文件
# =============================================================================


# =============================================================================
# 按模块组织源文件
# =============================================================================

# 递归收集所有头文件和源文件
file(GLOB_RECURSE HEADER_FILES "server/*.hpp")
file(GLOB_RECURSE SOURCE_FILES "server/*.cpp")

# 按物理文件夹分组源文件和头文件，VS/CLion/Xcode等IDE均支持
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/server PREFIX "server" FILES ${HEADER_FILES} ${SOURCE_FILES})

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
# 为头文件提供 server 根目录，支持 #include "core/xxx.hpp" 等写法
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/server)

# =============================================================================
# 链接库
# =============================================================================

target_link_libraries(${PROJECT_NAME} 
    Threads::Threads
)

# Windows特定库
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        ws2_32      # Winsock
        winmm       # Windows multimedia
    )
endif()

# =============================================================================
# 编译器特定设置
# =============================================================================

# MSVC特定设置
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _WINSOCK_DEPRECATED_NO_WARNINGS
    )
    
    # 设置运行时库
    set_property(TARGET ${PROJECT_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif()

# =============================================================================
# 安装配置
# =============================================================================

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# 安装头文件（按模块）
install(DIRECTORY server/core/ DESTINATION include/core FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY server/net/ DESTINATION include/net FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY server/utils/ DESTINATION include/utils FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY server/web/ DESTINATION include/web FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY server/monitor/ DESTINATION include/monitor FILES_MATCHING PATTERN "*.hpp")

# =============================================================================
# 打包配置
# =============================================================================

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "Mini Server Team")

include(CPack)

# =============================================================================
# 构建信息
# =============================================================================

message(STATUS "=== ${PROJECT_NAME} v${PROJECT_VERSION} ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Target platform: ${CMAKE_SYSTEM_NAME}")
if(WIN32)
    message(STATUS "Windows libraries: ws2_32, winmm")
endif()
message(STATUS "Source files and folders:")
foreach(src ${SOURCE_FILES})
    get_filename_component(_src_dir ${src} DIRECTORY)
    get_filename_component(_src_name ${src} NAME)
    message(STATUS "  ${_src_dir}/${_src_name}")
endforeach()
message(STATUS "Header files count: ${HEADER_FILES}")
message(STATUS "===========================================")

# =============================================================================
# 自定义目标
# =============================================================================

# 清理目标
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMENT "Cleaning all build files"
)

# 格式化代码目标（如果有clang-format）
find_program(CLANG_FORMAT_EXE clang-format)
if(CLANG_FORMAT_EXE)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i -style=Google ${MINISERVER_SOURCES} ${APP_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting source code with clang-format"
    )
endif()
