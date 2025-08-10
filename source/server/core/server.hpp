/**
 * @file server.hpp
 * @brief Core HTTP Server Class Definition
 * @author Mini Server Team
 * @version 1.0.0
 * @date 2024
 */

#pragma once

#include "service_registry.hpp"
#include "request_router.hpp"
#include "../net/socket_server.hpp"
#include "../net/http_types.hpp"

#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <functional>
#include <vector>

namespace miniserver::services
{
    class ServiceRegistry;
    struct ServiceInfo;
}

namespace miniserver::core
{
    /**
     * @brief Core HTTP Server
     *
     * Orchestrates HTTP request handling, service registration, and network communication.
     */
    class Server
    {
    public:
        /**
         * @brief Constructor
         * @param port Port number to bind to
         */
        explicit Server(int port = 8080);
        /**
         * @brief Destructor
         */
        ~Server();
        // Disable copy constructor and assignment
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
        /**
         * @brief Start the server
         * @throws std::runtime_error if server fails to start
         */
        void Start();
        /**
         * @brief Stop the server gracefully
         */
        void Stop();
        /**
         * @brief Check if server is running
         * @return True if server is running, false otherwise
         */
        bool IsRunning() const;
        /**
         * @brief Register a simple service with a body->string handler
         * @param name Service name
         * @param handler Function taking request body and returning response body (JSON string preferred)
         */
        void RegisterService(const std::string& name, std::function<std::string(const std::string&)> handler);
        /**
         * @brief Register a full HTTP service handler
         * @param name Service name
         * @param handler Handler taking full Request and returning full Response
         * @return true if registered successfully
         */
        bool RegisterService(const std::string& name, services::ServiceHandler handler);
        /**
         * @brief Unregister a previously registered service
         * @param name Service name
         * @return true if unregistered successfully
         */
        bool UnregisterService(const std::string& name);
        /**
         * @brief Get list of registered service names
         */
        std::vector<std::string> GetRegisteredServices() const;
    private:
        int m_port;                                                       ///< Server port
        std::atomic<bool> m_running;                                      ///< Running state flag
        std::thread m_server_thread;                                       ///< Server thread
        services::ServiceRegistry* m_service_registry;                     ///< Service registry (singleton)
        std::unique_ptr<RequestRouter> m_request_router;                   ///< Request router
        std::unique_ptr<network::SocketServer> m_socket_server;            ///< Network socket server
        /**
         * @brief Server main loop
         */
        void RunServer();
        /**
         * @brief Handle raw request string
         * @param raw_request Raw HTTP request string
         * @return HTTP response string
         */
        std::string HandleRequest(const std::string& raw_request);
        /**
         * @brief Handle incoming HTTP request
         * @param request The HTTP request to handle
         * @return HTTP response
         */
        http::Response HandleHttpRequest(const http::Request& request);
        /**
         * @brief Handle service requests (/service/<name>)
         * @param request The HTTP request
         * @param service_name Name of the service to call
         * @return HTTP response
         */
        http::Response HandleServiceRequest(const http::Request& request,
                                           const std::string& service_name);
    };
} // namespace miniserver::core
