/**
 * @file test_client.cpp
 * @brief Test Client for Mini Server
 * @author Mini Server Team
 * @version 1.0.0
 * @date 2024
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#endif

class HttpClient
{
private:
    std::string host_;
    int port_;

#ifdef _WIN32
    WSADATA wsa_data_;
    bool wsa_initialized_;
#endif

public:
    HttpClient(const std::string& host = "localhost", int port = 8080)
        : host_(host), port_(port)
    {
#ifdef _WIN32
        wsa_initialized_ = false;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data_) != 0)
        {
            throw std::runtime_error("Failed to initialize Winsock");
        }
        wsa_initialized_ = true;
#endif
    }

    ~HttpClient()
    {
#ifdef _WIN32
        if (wsa_initialized_)
        {
            WSACleanup();
        }
#endif
    }

    struct HttpResponse
    {
        int status_code;
        std::string status_text;
        std::string headers;
        std::string body;
        
        std::string ToString() const
        {
            std::ostringstream oss;
            oss << "HTTP Response:\n";
            oss << "Status: " << status_code << " " << status_text << "\n";
            oss << "Headers:\n" << headers;
            oss << "Body: " << body << "\n";
            return oss.str();
        }
    };

    HttpResponse SendRequest(const std::string& method, const std::string& path, 
                           const std::string& body = "", 
                           const std::string& content_type = "text/plain")
    {
        HttpResponse response;
        
        // Create socket
#ifdef _WIN32
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET)
#else
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
#endif
        {
            throw std::runtime_error("Failed to create socket");
        }

        try
        {
            // Setup server address
            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port_);

            // Convert hostname to IP
            struct hostent* server = gethostbyname(host_.c_str());
            if (server == nullptr)
            {
                throw std::runtime_error("Failed to resolve hostname: " + host_);
            }
            memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

            // Connect to server
            if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
            {
                throw std::runtime_error("Failed to connect to server");
            }

            // Build HTTP request
            std::ostringstream request_stream;
            request_stream << method << " " << path << " HTTP/1.1\r\n";
            request_stream << "Host: " << host_ << ":" << port_ << "\r\n";
            request_stream << "User-Agent: MiniServer-TestClient/1.0\r\n";
            request_stream << "Connection: close\r\n";
            
            if (!body.empty())
            {
                request_stream << "Content-Type: " << content_type << "\r\n";
                request_stream << "Content-Length: " << body.length() << "\r\n";
            }
            
            request_stream << "\r\n";
            
            if (!body.empty())
            {
                request_stream << body;
            }

            std::string request_str = request_stream.str();

            // Send request
#ifdef _WIN32
            if (send(sock, request_str.c_str(), static_cast<int>(request_str.length()), 0) == SOCKET_ERROR)
#else
            if (send(sock, request_str.c_str(), request_str.length(), 0) < 0)
#endif
            {
                throw std::runtime_error("Failed to send request");
            }

            // Receive response
            std::string response_str;
            char buffer[4096];
            int bytes_received;

#ifdef _WIN32
            while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0)
#else
            while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0)
#endif
            {
                buffer[bytes_received] = '\0';
                response_str += buffer;
            }

            // Parse response
            ParseHttpResponse(response_str, response);
        }
        catch (...)
        {
#ifdef _WIN32
            closesocket(sock);
#else
            close(sock);
#endif
            throw;
        }

#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif

        return response;
    }

private:
    void ParseHttpResponse(const std::string& response_str, HttpResponse& response)
    {
        std::istringstream iss(response_str);
        std::string line;

        // Parse status line
        if (std::getline(iss, line))
        {
            std::istringstream status_line(line);
            std::string http_version;
            status_line >> http_version >> response.status_code;
            std::getline(status_line, response.status_text);
            // Remove leading space and carriage return
            if (!response.status_text.empty() && response.status_text[0] == ' ')
            {
                response.status_text = response.status_text.substr(1);
            }
            if (!response.status_text.empty() && response.status_text.back() == '\r')
            {
                response.status_text.pop_back();
            }
        }

        // Parse headers
        std::ostringstream headers_stream;
        while (std::getline(iss, line) && line != "\r" && !line.empty())
        {
            headers_stream << line << "\n";
        }
        response.headers = headers_stream.str();

        // Parse body
        std::ostringstream body_stream;
        while (std::getline(iss, line))
        {
            body_stream << line << "\n";
        }
        response.body = body_stream.str();
        
        // Remove trailing newline if present
        if (!response.body.empty() && response.body.back() == '\n')
        {
            response.body.pop_back();
        }
    }
};

class TestClient
{
private:
    HttpClient client_;
    int total_tests_;
    int passed_tests_;
    int failed_tests_;

public:
    TestClient(const std::string& host = "localhost", int port = 8080)
        : client_(host, port), total_tests_(0), passed_tests_(0), failed_tests_(0)
    {
    }

    void RunAllTests()
    {
        std::cout << "\n=== Mini Server Test Client v1.0.0 ===\n" << std::endl;
        std::cout << "Starting comprehensive server tests...\n" << std::endl;

        // Test server connectivity first
        TestPing();
        
        // Test service listing
        TestGetServices();
        
        // Test all example services
        TestEchoService();
        TestUpperService();
        TestReverseService();
        TestLengthService();
        
        // Test error cases
        TestNonExistentService();
        TestInvalidMethod();
        
        // Print summary
        PrintTestSummary();
    }

private:
    void TestPing()
    {
        std::cout << "Testing /ping endpoint..." << std::endl;
        
        try
        {
            auto response = client_.SendRequest("GET", "/ping");
            
            if (response.status_code == 200)
            {
                std::cout << "✓ PASS: Ping test successful" << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(true);
            }
            else
            {
                std::cout << "✗ FAIL: Ping test failed with status " << response.status_code << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(false);
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "✗ FAIL: Ping test threw exception: " << e.what() << std::endl;
            RecordTest(false);
        }
        std::cout << std::endl;
    }

    void TestGetServices()
    {
        std::cout << "Testing /services endpoint..." << std::endl;
        
        try
        {
            auto response = client_.SendRequest("GET", "/services");
            
            if (response.status_code == 200)
            {
                std::cout << "✓ PASS: Get services test successful" << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(true);
            }
            else
            {
                std::cout << "✗ FAIL: Get services test failed with status " << response.status_code << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(false);
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "✗ FAIL: Get services test threw exception: " << e.what() << std::endl;
            RecordTest(false);
        }
        std::cout << std::endl;
    }

    void TestEchoService()
    {
        std::cout << "Testing /service/echo endpoint..." << std::endl;
        
        std::string test_input = "Hello, World!";
        
        try
        {
            auto response = client_.SendRequest("POST", "/service/echo", test_input);
            
            if (response.status_code == 200)
            {
                std::cout << "✓ PASS: Echo service test successful" << std::endl;
                std::cout << "  Input: " << test_input << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(true);
            }
            else
            {
                std::cout << "✗ FAIL: Echo service test failed with status " << response.status_code << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(false);
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "✗ FAIL: Echo service test threw exception: " << e.what() << std::endl;
            RecordTest(false);
        }
        std::cout << std::endl;
    }

    void TestUpperService()
    {
        std::cout << "Testing /service/upper endpoint..." << std::endl;
        
        std::string test_input = "hello world";
        
        try
        {
            auto response = client_.SendRequest("POST", "/service/upper", test_input);
            
            if (response.status_code == 200)
            {
                std::cout << "✓ PASS: Upper service test successful" << std::endl;
                std::cout << "  Input: " << test_input << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(true);
            }
            else
            {
                std::cout << "✗ FAIL: Upper service test failed with status " << response.status_code << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(false);
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "✗ FAIL: Upper service test threw exception: " << e.what() << std::endl;
            RecordTest(false);
        }
        std::cout << std::endl;
    }

    void TestReverseService()
    {
        std::cout << "Testing /service/reverse endpoint..." << std::endl;
        
        std::string test_input = "12345";
        
        try
        {
            auto response = client_.SendRequest("POST", "/service/reverse", test_input);
            
            if (response.status_code == 200)
            {
                std::cout << "✓ PASS: Reverse service test successful" << std::endl;
                std::cout << "  Input: " << test_input << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(true);
            }
            else
            {
                std::cout << "✗ FAIL: Reverse service test failed with status " << response.status_code << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(false);
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "✗ FAIL: Reverse service test threw exception: " << e.what() << std::endl;
            RecordTest(false);
        }
        std::cout << std::endl;
    }

    void TestLengthService()
    {
        std::cout << "Testing /service/length endpoint..." << std::endl;
        
        std::string test_input = "test string";
        
        try
        {
            auto response = client_.SendRequest("POST", "/service/length", test_input);
            
            if (response.status_code == 200)
            {
                std::cout << "✓ PASS: Length service test successful" << std::endl;
                std::cout << "  Input: " << test_input << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(true);
            }
            else
            {
                std::cout << "✗ FAIL: Length service test failed with status " << response.status_code << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(false);
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "✗ FAIL: Length service test threw exception: " << e.what() << std::endl;
            RecordTest(false);
        }
        std::cout << std::endl;
    }

    void TestNonExistentService()
    {
        std::cout << "Testing non-existent service (should return 404)..." << std::endl;
        
        try
        {
            auto response = client_.SendRequest("POST", "/service/nonexistent", "test");
            
            if (response.status_code == 404)
            {
                std::cout << "✓ PASS: Non-existent service correctly returned 404" << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(true);
            }
            else
            {
                std::cout << "✗ FAIL: Non-existent service returned unexpected status " << response.status_code << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(false);
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "✗ FAIL: Non-existent service test threw exception: " << e.what() << std::endl;
            RecordTest(false);
        }
        std::cout << std::endl;
    }

    void TestInvalidMethod()
    {
        std::cout << "Testing invalid method on service endpoint..." << std::endl;
        
        try
        {
            auto response = client_.SendRequest("GET", "/service/echo");
            
            if (response.status_code == 405 || response.status_code == 400)
            {
                std::cout << "✓ PASS: Invalid method correctly returned " << response.status_code << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(true);
            }
            else
            {
                std::cout << "✗ FAIL: Invalid method returned unexpected status " << response.status_code << std::endl;
                std::cout << "  Response: " << response.body << std::endl;
                RecordTest(false);
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "✗ FAIL: Invalid method test threw exception: " << e.what() << std::endl;
            RecordTest(false);
        }
        std::cout << std::endl;
    }

    void RecordTest(bool passed)
    {
        total_tests_++;
        if (passed)
        {
            passed_tests_++;
        }
        else
        {
            failed_tests_++;
        }
    }

    void PrintTestSummary()
    {
        std::cout << "=== Test Summary ===" << std::endl;
        std::cout << "Total tests: " << total_tests_ << std::endl;
        std::cout << "Passed: " << passed_tests_ << std::endl;
        std::cout << "Failed: " << failed_tests_ << std::endl;
        
        double success_rate = total_tests_ > 0 ? (double)passed_tests_ / total_tests_ * 100.0 : 0.0;
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "Success rate: " << success_rate << "%" << std::endl;
        
        if (failed_tests_ == 0)
        {
            std::cout << "\n🎉 All tests passed! Server is working correctly." << std::endl;
        }
        else
        {
            std::cout << "\n⚠️  Some tests failed. Please check the server implementation." << std::endl;
        }
    }
};

int main(int argc, char* argv[])
{
    std::string host = "localhost";
    int port = 8080;

    // Parse command line arguments
    if (argc > 1)
    {
        host = argv[1];
    }
    if (argc > 2)
    {
        try
        {
            port = std::stoi(argv[2]);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Invalid port number: " << argv[2] << std::endl;
            return 1;
        }
    }

    std::cout << "Connecting to server at " << host << ":" << port << std::endl;

    try
    {
        TestClient test_client(host, port);
        test_client.RunAllTests();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
