@echo off
REM Quick test runner script
REM This script assumes the test client has already been built

echo Mini Server Test Client - Quick Runner
echo ========================================

REM Check if executable exists
if exist build\Debug\test_client.exe (
    echo Found test client executable, running tests...
    echo.
    build\Debug\test_client.exe
) else (
    echo Test client not found! Please build it first by running build_and_run.bat
    echo.
    pause
    exit /b 1
)

echo.
echo Tests completed. Press any key to exit...
pause > nul
