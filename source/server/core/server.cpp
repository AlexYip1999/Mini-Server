/**
 * @file server.cpp
 * @brief Implementation of the core HTTP server for MiniServer.
 * @author Mini Server Team
 * @version 1.0.0
 * @date 2024
 */

#include "service_registry.hpp"
#include "server.hpp"
// #include "request_router.hpp"  // Request router is currently disabled
#include "../net/http_types.hpp"    // Ensure proper namespace resolution
#include "../net/socket_server.hpp"
#include "../net/http_parser.hpp"
#include "../utils/logger.hpp"

#include <stdexcept>
#include <algorithm>
#include <vector>

namespace miniserver::core
{

    /**
     * @brief Construct a new Server object
     * @param port Port number to bind the server
     * @throws std::invalid_argument if port is out of range
     */
    Server::Server(int port)
        : m_port(port)
        , m_service_registry(&services::ServiceRegistry::GetInstance())
        , m_request_router(nullptr)  // Request router is currently disabled
        , m_socket_server(std::make_unique<network::SocketServer>())
    {
        if (port <= 0 || port > 65535)
        {
            throw std::invalid_argument("Port must be between 1 and 65535");
        }
        LOG_INFO_FMT("Server", "Server created on port {}", m_port);
    }

    /**
     * @brief Destroy the Server object and stop the server if running
     */
    Server::~Server()
    {
        Stop();
    }

    /**
     * @brief Start the server in a new thread
     */
    void Server::Start()
    {
        if (m_running.load())
        {
            LOG_WARN("Server", "Server is already running");
            return;
        }
        LOG_INFO_FMT("Server", "Starting server on port {}", m_port);

        m_running.store(true);

        m_server_thread = std::thread(&Server::RunServer, this);
    }

    /**
     * @brief Stop the server and join the server thread
     */
    void Server::Stop()
    {
        if (!m_running.load())
        {
            return;
        }
        LOG_INFO("Server", "Stopping server...");

        m_running.store(false);

        if (m_socket_server)
        {
            m_socket_server->Stop();
        }

        if (m_server_thread.joinable())
        {
            m_server_thread.join();
        }

        LOG_INFO("Server", "Server stopped");
    }

    /**
     * @brief Register a service with a full HTTP handler
     * @param service_name Name of the service
     * @param handler Service handler function
     * @return true if registration succeeded, false otherwise
     */
    bool Server::RegisterService(const std::string& service_name, services::ServiceHandler handler)
    {
        if (m_running.load())
        {
            LOG_WARN_FMT("Server", "Cannot register service '{}': server is running", service_name);
            return false;
        }

        if (service_name.empty())
        {
            LOG_WARN("Server", "Cannot register service with empty name");
            return false;
        }

        if (!handler)
        {
            LOG_WARN_FMT("Server", "Cannot register service '{}': handler is null", service_name);
            return false;
        }

        services::ServiceInfo service_info(
            service_name + " service",
            "1.0.0",
            handler,
            true
        );

        bool success = m_service_registry->RegisterService(service_name, service_info);
        if (success) 
        {
            LOG_INFO_FMT("Server", "Service '{}' registered successfully", service_name);
        }
        else 
        {
            LOG_WARN_FMT("Server", "Failed to register service '{}': name already exists", service_name);
        }
        return success;
    }

    /**
     * @brief Register a service with a simple body-to-string handler
     * @param service_name Name of the service
     * @param body_handler Function taking request body and returning response body
     */
    void Server::RegisterService(const std::string& service_name, std::function<std::string(const std::string&)> body_handler)
    {
        services::ServiceHandler full_handler = [body_handler](const http::Request& request) -> http::Response
        {
            http::Response resp;
            if (!body_handler) 
            {
                resp.status = http::StatusCode::InternalServerError;
                resp.SetJson("{\"error\":\"Handler not set\"}");
                return resp;
            }
            try 
            {
                std::string out_body = body_handler(request.body);
                resp.status = http::StatusCode::OK;
                resp.headers["Content-Type"] = "application/json";
                resp.body = out_body;
            }
            catch (const std::exception& ex) 
            {
                resp.status = http::StatusCode::InternalServerError;
                resp.SetJson(std::string("{\"error\":\"Exception: ") + ex.what() + "\"}");
            }
            return resp;
        };

        (void)RegisterService(service_name, full_handler); // ignore bool for legacy void API
    }

    /**
     * @brief Unregister a service by name
     * @param service_name Name of the service
     * @return true if unregistration succeeded, false otherwise
     */
    bool Server::UnregisterService(const std::string& service_name)
    {
        if (m_running.load()) 
        {
            LOG_WARN_FMT("Server", "Cannot unregister service '{}': server is running", service_name);
            return false;
        }

        bool success = m_service_registry->UnregisterService(service_name);
        if (success) 
        {
            LOG_INFO_FMT("Server", "Service '{}' unregistered successfully", service_name);
        }
        else 
        {
            LOG_WARN_FMT("Server", "Failed to unregister service '{}': not found", service_name);
        }
        return success;
    }

    /**
     * @brief Get a list of all registered service names
     * @return Vector of service names
     */
    std::vector<std::string> Server::GetRegisteredServices() const
    {
        return m_service_registry->GetServiceNames();
    }

    /**
     * @brief Check if the server is currently running
     * @return true if running, false otherwise
     */
    bool Server::IsRunning() const
    {
        return m_running.load();
    }

    /**
     * @brief Main server loop, runs in a separate thread
     */
    void Server::RunServer()
    {
        try
        {
            // Start the socket server
            if (!m_socket_server->Start("0.0.0.0", m_port))
            {
                LOG_ERROR_FMT("Server", "Failed to start server on port {}", m_port);
                m_running.store(false);
                return;
            }
            LOG_INFO_FMT("Server", "Server running on http://localhost:{}", m_port);

            // Run the server with our request handler
            m_socket_server->Run([this](const std::string& request_data) -> std::string
            {
                return HandleRequest(request_data);
            });
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_FMT("Server", "Server error: {}", e.what());
            m_running.store(false);
        }   
    }

    /**
     * @brief Handle a raw HTTP request string and return a serialized response
     * @param request_data Raw HTTP request string
     * @return Serialized HTTP response string
     */
    std::string Server::HandleRequest(const std::string& request_data)
    {
        try
        {
            LOG_DEBUG_FMT("Server", "Received request: {} bytes", request_data.size());
            // Parse HTTP request
            auto request_opt = http::HttpParser::ParseRequest(request_data);
            if (!request_opt)
            {
                LOG_WARN("Server", "Received invalid HTTP request");
                http::Response error_response;
                error_response.status = http::StatusCode::BadRequest;
                error_response.SetText("Bad Request");
                return http::HttpParser::SerializeResponse(error_response);
            }
            const auto& request = *request_opt;
            LOG_DEBUG_FMT("Server", "Processing {} request to {}",
                         http::MethodToString(request.method), request.path);
            http::Response response;
            // Handle CORS preflight requests
            if (request.method == http::Method::OPTIONS)
            {
                response.status = http::StatusCode::OK;
                response.headers["Access-Control-Allow-Origin"] = "*";
                response.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
                response.headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
                response.headers["Access-Control-Max-Age"] = "86400";
                response.body = "";
            }
            // Health check endpoint
            else if (request.path == "/ping" && request.method == http::Method::GET) {
                response.status = http::StatusCode::OK;
                response.SetJson("{\"status\":\"ok\",\"message\":\"pong\"}");
            }
            // List services endpoint
            else if (request.path == "/services" && request.method == http::Method::GET)
            {
                response = m_service_registry->GetServicesInfo();
            }
            // Service invocation endpoint /service/<name>
            else if (request.path.substr(0, 9) == "/service/" && request.method == http::Method::POST)
            {
                std::string service_name = request.path.substr(9); // Remove "/service/"
                if (!service_name.empty())
                {
                    response = m_service_registry->HandleServiceRequest(request, service_name);
                }
                else
                {
                    response.status = http::StatusCode::BadRequest;
                    response.SetJson("{\"error\":\"Service name is required\"}");
                }
            }
            // Default response for unknown endpoints
            else
            {
                response.status = http::StatusCode::NotFound;
                response.SetJson("{\"error\":\"Endpoint not found\"}");
            }
            // Add CORS headers to all responses
            response.headers["Access-Control-Allow-Origin"] = "*";
            response.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
            response.headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
            // Return serialized response
            return http::HttpParser::SerializeResponse(response);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_FMT("Server", "Error handling request: {}", e.what());
            http::Response error_response;
            error_response.status = http::StatusCode::InternalServerError;
            error_response.SetJson("{\"error\":\"Internal Server Error\"}");
            return http::HttpParser::SerializeResponse(error_response);
        }
    }

} // namespace core

