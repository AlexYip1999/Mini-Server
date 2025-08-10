@echo off
echo Building Mini Server Test Client...

REM Create build directory
if not exist build mkdir build
cd build

REM Configure CMake
cmake .. -G "Visual Studio 17 2022" -A x64

REM Build the project
cmake --build . --config Debug

REM Check if build was successful
if exist Debug\test_client.exe (
    echo.
    echo Build successful! Test client executable created: build\Debug\test_client.exe
    echo.
    echo Usage:
    echo   test_client.exe [host] [port]
    echo.
    echo Examples:
    echo   test_client.exe                    ^(connects to localhost:8080^)
    echo   test_client.exe localhost 8080
    echo   test_client.exe 192.168.1.100 9090
    echo.
    echo To run tests now, make sure the Mini Server is running first, then run:
    echo   Debug\test_client.exe
) else (
    echo.
    echo Build failed! Please check the error messages above.
    pause
    exit /b 1
)

cd ..
pause
