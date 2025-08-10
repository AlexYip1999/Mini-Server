/**
 * @file main.cpp
 * @brief Application Entry Point
 * @author Mini Server Team
 * @version 1.0.0
 */

#include "core/server.hpp"
#include "utils/logger.hpp"

#include <iostream>
#include <signal.h>
#include <memory>
#include <thread>
#include <chrono>
#include <algorithm>

// Global server instance pointer
std::unique_ptr<miniserver::core::Server> g_server;

/**
 * @brief Signal handler for graceful shutdown
 * @param signal Signal value
 */
void SignalHandler(int signal)
{
    LOG_INFO_FMT("Main", "Received signal {} , shutting down...", signal);
    if (g_server)
    {
        g_server->Stop();
    }
    exit(0);
}

/**
 * @brief Register example services to the server
 * @param server Server instance
 */
void RegisterExampleServices(miniserver::core::Server& server)
{
    LOG_INFO("Main", "Registering example services");

    // Echo service: returns input as output
    server.RegisterService("echo", [](const std::string& body) -> std::string 
    {
        return R"({"service":"echo","input":")" + body + R"(","output":")" + body + R"("})";
    });

    // Upper service: converts input to uppercase
    server.RegisterService("upper", [](const std::string& body) -> std::string 
    {
        std::string upper_body = body;
        std::transform(upper_body.begin(), upper_body.end(), upper_body.begin(), [](unsigned char c)
        {
            return static_cast<char>(std::toupper(c));
        });

        return R"({"service":"upper","input":")" + body + R"(","output":")" + upper_body + R"("})";
    });
    // Reverse service: reverses input string
    server.RegisterService("reverse", [](const std::string& body) -> std::string 
    {
        std::string reversed_body = body;
        std::reverse(reversed_body.begin(), reversed_body.end());
        return R"({"service":"reverse","input":")" + body + R"(","output":")" + reversed_body + R"("})";
    });

    // Length service: returns length of input string
    server.RegisterService("length", [](const std::string& body) -> std::string 
    {
        return R"({"service":"length","input":")" + body + R"(","length":)" + std::to_string(body.length()) + R"(})";
    });

    LOG_INFO("Main", "Example services registered: echo, upper, reverse, length");
}

/**
 * @brief Print usage information to console
 */
void PrintUsage()
{
    std::cout << "\n=== Mini Server v1.0.0 ===" << std::endl;
    std::cout << "A lightweight HTTP server with service registration capabilities\n" << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  GET  /ping              - Health check" << std::endl;
    std::cout << "  GET  /services          - List registered services" << std::endl;
    std::cout << "  POST /service/<name>    - Call a specific service" << std::endl;
    std::cout << "  OPTIONS /*              - CORS preflight" << std::endl;
    std::cout << "\nExample services:" << std::endl;
    std::cout << "  POST /service/echo      - Echo input back" << std::endl;
    std::cout << "  POST /service/upper     - Convert to uppercase" << std::endl;
    std::cout << "  POST /service/reverse   - Reverse string" << std::endl;
    std::cout << "  POST /service/length    - Get string length" << std::endl;
    std::cout << "\nExample usage:" << std::endl;
    std::cout << "  curl -X GET http://localhost:8080/ping" << std::endl;
    std::cout << "  curl -X POST http://localhost:8080/service/echo -d \"Hello World\"" << std::endl;
    std::cout << "  curl -X POST http://localhost:8080/service/upper -d \"hello\"" << std::endl;
    std::cout << "\nPress Ctrl+C to stop the server" << std::endl;
}

/**
 * @brief Main entry point
 * @param argc Argument count
 * @param argv Argument values
 * @return Exit code
 */
int Main(int argc, char* argv[])
{
    try
    {
        miniserver::utils::Logger::GetInstance().SetLogLevel(miniserver::utils::LogLevel::Info);
        int port = 8080;
        if (argc > 1)
        {
            try
            {
                port = std::stoi(argv[1]);
                if (port <= 0 || port > 65535)
                {
                    throw std::out_of_range("Port out of range");
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Invalid port number: " << argv[1] << "\n";
                std::cerr << "Usage: " << argv[0] << " [port]" << "\n";
                std::cerr << "Error: " << e.what() << "\n";
                return 1;
            }
        }

        signal(SIGINT, SignalHandler);
        signal(SIGTERM, SignalHandler);

        g_server = std::make_unique<miniserver::core::Server>(port);
        RegisterExampleServices(*g_server);
        g_server->Start();

        PrintUsage();

        while (g_server->IsRunning())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR_FMT("Main", "Fatal error: {}", e.what());
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    LOG_INFO("Main", "Application exiting");
    return 0;
}

// For compatibility with C++ main signature
int main(int argc, char* argv[])
{
    return Main(argc, argv);
}
