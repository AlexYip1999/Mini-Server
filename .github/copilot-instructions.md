<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

# MiniServer C++ Project Instructions

This is a C++ HTTP server project for system monitoring with web interface and virtual character features.

## Project Context
- **Language**: C++17
- **Build System**: CMake
- **HTTP Library**: cpp-httplib (lightweight header-only library)
- **Platform**: Windows (with Windows API for system monitoring)
- **Architecture**: Multi-threaded server with REST API

## Key Components
1. **Server Class**: HTTP server wrapper using cpp-httplib
2. **SystemMonitor Class**: Windows API-based system information gathering
3. **WebHandler Class**: Static file serving and HTML template generation
4. **Main Application**: Signal handling and server lifecycle management

## Code Style Guidelines
- Use modern C++17 features
- Follow RAII principles
- Use smart pointers where appropriate
- Implement proper error handling
- Use const-correctness
- Follow Google C++ Style Guide naming conventions

## Platform-Specific Notes
- Uses Windows API (GetSystemTimes, GlobalMemoryStatusEx, GetDiskFreeSpaceEx)
- Requires linking against ws2_32, winmm, pdh, wbemuuid, ole32, oleaut32
- PDH (Performance Data Helper) for performance counters

## Web Interface Features
- Responsive design with CSS Grid/Flexbox
- Real-time data updates via REST API
- Virtual character with animated expressions
- Progress bars and metrics visualization
- Mobile-friendly responsive layout

## Dependencies
- cpp-httplib: Single-header HTTP library
- Windows SDK: For system API access
- CMake 3.15+: Build system

## When implementing new features:
1. Follow the existing class structure
2. Add appropriate error handling
3. Update both C++ backend and HTML frontend
4. Consider cross-platform compatibility for future expansion
5. Maintain the real-time update mechanism
