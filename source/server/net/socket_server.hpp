/**
 * @file socket_server.hpp
 * @brief Cross-platform network socket server.
 *
 * Provides a lightweight cross-platform (Windows / Unix-like) TCP server used
 * as the transport for the HTTP layer in MiniServer. It wraps low level
 * socket operations and exposes a unified interface.
 */

#pragma once

#include <atomic>
#include <string>
#include <functional>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    using SOCKET = int;
    const SOCKET INVALID_SOCKET = -1;
    const int SOCKET_ERROR = -1;
#endif

namespace miniserver::network {

// Request handler functor: takes raw request data, returns serialized response
using RequestHandler = std::function<std::string(const std::string& request_data)>;

/**
 * @brief Cross-platform network socket server
 * 
 * Provides a lightweight cross-platform (Windows / Unix-like) TCP server used
 * as the transport for the HTTP layer in MiniServer. It wraps low level
 * socket operations and exposes a unified interface.
 * 
 * @details
 * This class implements a simple TCP server that can be used to accept
 * incoming HTTP requests and dispatch them to a handler function.
 * It handles the low-level details of socket programming, allowing
 * the user to focus on request handling logic.
 * 
 * @example
 * @code
 * SocketServer server;
 * if (server.start("0.0.0.0", 8080)) {
 *     server.run([](const std::string& request) -> std::string {
 *         return "HTTP/1.1 200 OK\r\n\r\nHello World";
 *     });
 * }
 * @endcode
 */
class SocketServer {
public:
    /**
     * @brief Construct and (on Windows) initialize Winsock
     * @throws std::runtime_error If network initialization fails
     */
    SocketServer();
    
    /**
     * @brief RAII cleanup
     */
    ~SocketServer();

    // Disable copy and move operations
    SocketServer(const SocketServer&) = delete;
    SocketServer& operator=(const SocketServer&) = delete;
    SocketServer(SocketServer&&) = delete;
    SocketServer& operator=(SocketServer&&) = delete;

    /**
     * @brief Start server (bind + listen). host "0.0.0.0" binds all interfaces.
     * @param host Binding IP address, default "0.0.0.0" for all interfaces
     * @param port Listening port
     * @return true if started successfully
     * 
     * @details
     * Creates a socket, binds it to the specified address and port,
     * and starts listening for incoming connections.
     * If host is empty or "0.0.0.0", it will bind to all available interfaces.
     */
    bool Start(const std::string& host, int port);
    
    /**
     * @brief Stop server (close listening socket)
     * 
     * @details
     * Closes the listening socket and stops accepting new connections.
     * Established connections will be closed naturally.
     */
    void Stop();
    
    /**
     * @brief Accept loop (spawns a detached thread per client)
     * @param handler Request handler function
     * 
     * @details
     * Starts accepting client connections and processing requests.
     * This method will block until the server is stopped.
     * Each client connection is handled in a separate thread.
     */
    void Run(RequestHandler handler);
    
    /**
     * @brief Running state
     * @return true if the server is running
     */
    bool IsRunning() const;
    
    /**
     * @brief Return host:port if running else empty string
     * @return Server address string (host:port format)
     */
    std::string GetAddress() const;

private:
    /**
     * @brief Handle a single client connection
     * @param client_socket Client socket
     * @param handler Request handler
     * @param client_ip Client IP address
     */
    void HandleClient(SOCKET client_socket, RequestHandler handler, const std::string& client_ip);
    
    /**
     * @brief Receive data from client
     * @param client_socket Client socket
     * @return Received data
     */
    std::string ReceiveData(SOCKET client_socket);
    
    /**
     * @brief Send data to client
     * @param client_socket Client socket
     * @param data Data to send
     * @return true if sent successfully
     */
    bool SendData(SOCKET client_socket, const std::string& data);
    
    /**
     * @brief Set socket options
     * @return true if set successfully
     */
    bool SetSocketOptions();
    
    /**
     * @brief Set client socket timeout
     * @param client_socket Client socket
     * @param timeout_seconds Timeout duration (seconds)
     */
    void SetClientSocketTimeout(SOCKET client_socket, int timeout_seconds);
    
    /**
     * @brief Close socket
     * @param socket Socket to close
     */
    static void CloseSocket(SOCKET socket);
    
    /**
     * @brief Get last error string
     * @return Error information string
     */
    static std::string GetLastErrorString();

private:
    SOCKET m_server_socket;                     ///< Server socket descriptor
    std::atomic<bool> m_is_running;             ///< Server running state
    std::string m_host;                         ///< Bound host address
    int m_port;                                 ///< Listening port
};

} // namespace miniserver::network

