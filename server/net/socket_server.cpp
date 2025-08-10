/**
 * @file socket_server.cpp
 * @brief Cross-platform socket server implementation
 */

#include "net/socket_server.hpp"
#include "utils/logger.hpp"
#include <iostream>
#include <chrono>
#include <thread>

#ifdef _WIN32
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
#endif

namespace miniserver::network
{

SocketServer::SocketServer()
    : m_server_socket(INVALID_SOCKET)
    , m_is_running(false)
{
#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
    utils::Logger::GetInstance().Error(
            "SocketServer", "WSAStartup failed: " + std::to_string(result));
        throw std::runtime_error("Unable to initialize Winsock");
    }
#endif

    utils::Logger::GetInstance().Error(
        "SocketServer", "Socket server initialized");
}

SocketServer::~SocketServer()
{
    Stop();
#ifdef _WIN32
    WSACleanup();
#endif
    utils::Logger::GetInstance().Info("SocketServer", "Socket server destroyed");
}

bool SocketServer::Start(const std::string& host, int port)
{
    if (IsRunning())
    {
        utils::Logger::GetInstance().Warning("SocketServer", 
            "Server is already running");
        return false;
    }

    // Create socket
    m_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_server_socket == INVALID_SOCKET)
    {
        utils::Logger::GetInstance().Error("SocketServer",
            "Failed to create socket: " + GetLastErrorString());
        return false;
    }

    // Set socket options
    if (!SetSocketOptions())
    {
        CloseSocket(m_server_socket);
        m_server_socket = INVALID_SOCKET;
        return false;
    }

    // Bind address
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(static_cast<uint16_t>(port));

    if (host == "0.0.0.0" || host.empty())
    {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0)
        {
            utils::Logger::GetInstance().Error(
                "SocketServer", "Invalid IP address: " + host);
            CloseSocket(m_server_socket);
            m_server_socket = INVALID_SOCKET;
            return false;
        }
    }

    if (bind(m_server_socket, reinterpret_cast<struct sockaddr*>(&server_addr),
             sizeof(server_addr)) == SOCKET_ERROR)
    {
        utils::Logger::GetInstance().Error(
            "SocketServer", "Failed to bind address: " + GetLastErrorString());
        CloseSocket(m_server_socket);
        m_server_socket = INVALID_SOCKET;
        return false;
    }

    // Start listening
    if (listen(m_server_socket, SOMAXCONN) == SOCKET_ERROR)
    {
        utils::Logger::GetInstance().Error(
            "SocketServer", "Listen failed: " + GetLastErrorString());
        CloseSocket(m_server_socket);
        m_server_socket = INVALID_SOCKET;
        return false;
    }

    m_host = host;
    m_port = port;

    m_is_running.store(true);

    utils::Logger::GetInstance().Error(
        "SocketServer", "Server started successfully at " + host + ":" + std::to_string(port));
    return true;
}

void SocketServer::Stop()
{
    if (!IsRunning())
    {
        return;
    }

    m_is_running.store(false);

    if (m_server_socket != INVALID_SOCKET)
    {
        CloseSocket(m_server_socket);
        m_server_socket = INVALID_SOCKET;
    }

    utils::Logger::GetInstance().Error(
        "SocketServer", "Server stopped");
}

void SocketServer::Run(RequestHandler handler)
{
    if (!IsRunning() || !handler)
    {
        utils::Logger::GetInstance().Error("SocketServer", 
            "Server not running or handler is null");
        return;
    }

    utils::Logger::GetInstance().Error(
        "SocketServer", 
        "Waiting for client connections...");

    while (IsRunning())
    {
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        SOCKET client_socket = accept(
            m_server_socket,
            reinterpret_cast<struct sockaddr*>(&client_addr),
            &client_addr_len);

        if (client_socket == INVALID_SOCKET)
        {
            if (IsRunning())
            {
                utils::Logger::GetInstance().Error("SocketServer", 
                    "Accept failed: " + GetLastErrorString());

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            continue;
        }

        // Get client address
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

        utils::Logger::GetInstance().Error("SocketServer", 
            "Accepted connection from " + std::string(client_ip));

        // Handle client in a new thread
        std::thread client_thread([this, client_socket, handler, client_ip]()
        {
            HandleClient(client_socket, handler, client_ip);
        });
        client_thread.detach();
    }
}

bool SocketServer::IsRunning() const
{
    return m_is_running.load();
}

std::string SocketServer::GetAddress() const
{
    if (!IsRunning())
    {
        return "";
    }
    return m_host + ":" + std::to_string(m_port);
}

void SocketServer::HandleClient(SOCKET client_socket,
                                RequestHandler handler,
                                const std::string& client_ip)
{
    try
    {
        // Set client socket timeout
        SetClientSocketTimeout(client_socket, 30); // 30 seconds

        // Receive request
        const std::string request_data = ReceiveData(client_socket);
        if (request_data.empty())
        {
            utils::Logger::GetInstance().Error(
                "SocketServer", "No data received from " + client_ip);

            CloseSocket(client_socket);
            return;
        }

        utils::Logger::GetInstance().Error(
            "SocketServer", "Received " + std::to_string(request_data.size()) +
            " bytes from " + client_ip);

        // Process request
        std::string response = handler(request_data);

        // Send response
        if (!SendData(client_socket, response))
        {
            utils::Logger::GetInstance().Error(
                "SocketServer", "Failed to send response to " + client_ip);
        }
    }
    catch (const std::exception& e)
    {
        utils::Logger::GetInstance().Error(
            "SocketServer", "Exception while handling client " + client_ip + ": " + e.what());
    }

    CloseSocket(client_socket);
}

std::string SocketServer::ReceiveData(SOCKET client_socket)
{
    std::string data;
    char buffer[8192];
    int total_received = 0;
    bool headers_complete = false;
    size_t content_length = 0;
    size_t headers_end_pos = 0;

    while (true)
    {
        const int received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0)
        {
            if (received == 0)
            {
                utils::Logger::GetInstance().Error(
                    "SocketServer", "Client closed connection");
            }
            else
            {
                utils::Logger::GetInstance().Error(
                    "SocketServer", "Error receiving data: " + GetLastErrorString());
            }
            break;
        }

        buffer[received] = '\0';
        data.append(buffer, received);
        total_received += received;

        // Detect end of headers
        if (!headers_complete)
        {
            headers_end_pos = data.find("\r\n\r\n");
            if (headers_end_pos != std::string::npos)
            {
                headers_complete = true;
                headers_end_pos += 4; // Skip delimiter

                // Parse Content-Length
                auto content_length_pos = data.find("Content-Length: ");
                if (content_length_pos != std::string::npos)
                {
                    content_length_pos += 16; // skip label
                    const auto line_end = data.find("\r\n", content_length_pos);
                    if (line_end != std::string::npos)
                    {
                        std::string length_str = data.substr(content_length_pos,
                                                             line_end - content_length_pos);
                        try
                        {
                            content_length = std::stoul(length_str);
                        }
                        catch (...)
                        {
                            content_length = 0;
                        }
                    }
                }
            }
        }

        // If we know expected body length, wait until complete
        if (headers_complete && content_length > 0)
        {
            const size_t current_body_size = data.size() - headers_end_pos;
            if (current_body_size >= content_length)
            {
                break; // complete
            }
        }
        else if (headers_complete && content_length == 0)
        {
            break; // no body
        }

        // Size guard (1MB)
        if (total_received > 1 * 1024 * 1024)
        {
            utils::Logger::GetInstance().Error(
                "SocketServer", "Received data too large, forcibly closing");
            break;
        }
    }

    return data;
}

bool SocketServer::SendData(SOCKET client_socket, const std::string& data)
{
    size_t total_sent = 0;
    size_t data_size = data.size();
    while (total_sent < data_size)
    {
        int sent = send(client_socket,
                        data.c_str() + total_sent,
                        static_cast<int>(data_size - total_sent), 0);
        if (sent == SOCKET_ERROR)
        {
            utils::Logger::GetInstance().Error(
                "SocketServer", "Send failed: " + GetLastErrorString());
            return false;
        }
        total_sent += sent;
    }
    return true;
}

bool SocketServer::SetSocketOptions()
{
    int reuse = 1;
    if (setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char*>(&reuse), sizeof(reuse)) == SOCKET_ERROR)
    {
        utils::Logger::GetInstance().Error(
            "SocketServer", "Failed to set SO_REUSEADDR: " + GetLastErrorString());
        return false;
    }
#ifdef _WIN32
    DWORD timeout = 30000; // 30s
    setsockopt(m_server_socket, SOL_SOCKET, SO_RCVTIMEO,
        reinterpret_cast<const char*>(&timeout), sizeof(timeout));
    setsockopt(m_server_socket, SOL_SOCKET, SO_SNDTIMEO,
        reinterpret_cast<const char*>(&timeout), sizeof(timeout));
#else
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    setsockopt(m_server_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(m_server_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
#endif
    return true;
}

void SocketServer::SetClientSocketTimeout(SOCKET client_socket, int timeout_seconds)
{
#ifdef _WIN32
    DWORD timeout = timeout_seconds * 1000; // ms
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO,
        reinterpret_cast<const char*>(&timeout), sizeof(timeout));
    setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO,
        reinterpret_cast<const char*>(&timeout), sizeof(timeout));
#else
    struct timeval timeout;
    timeout.tv_sec = timeout_seconds;
    timeout.tv_usec = 0;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
#endif
}

void SocketServer::CloseSocket(SOCKET socket)
{
    if (socket != INVALID_SOCKET)
    {
#ifdef _WIN32
        closesocket(socket);
#else
        close(socket);
#endif
    }
}

std::string SocketServer::GetLastErrorString()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    char* message = nullptr;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<char*>(&message), 0, nullptr);
    std::string result = message ? message : "Unknown error";
    if (message) LocalFree(message);
    return result + " (Error code: " + std::to_string(error) + ")";
#else
    return std::string(strerror(errno)) + " (Error code: " + std::to_string(errno) + ")";
#endif
}

} // namespace miniserver::network

