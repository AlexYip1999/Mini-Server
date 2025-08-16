/**
 * @file server.cpp
 * @brief Implementation of the core HTTP server for MiniServer.
 * @author Mini Server Team
 * @version 1.0.0
 * @date 2024
 */

#include "server.hpp"
#include "request_router.hpp"
#include "net/http_types.hpp"    // Ensure proper namespace resolution
#include "net/socket_server.hpp"
#include "net/http_parser.hpp"
#include "utils/logger.hpp"

#include <stdexcept>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace miniserver::core
{

    /**
     * @brief Construct a new Server object
     * @param port Port number to bind the server
     * @param web_root Web root directory for static files
     * @throws std::invalid_argument if port is out of range
     */
    Server::Server(int port, const std::string& web_root)
        : m_port(port)
        , m_service_registry(std::make_unique<services::ServiceRegistry>())
        , m_request_router(std::make_unique<RequestRouter>(m_service_registry.get(), web_root))
        , m_socket_server(std::make_unique<network::SocketServer>())
    {
        if (port <= 0 || port > 65535)
        {
            throw std::invalid_argument("Port must be between 1 and 65535");
        }
        LOG_INFO_FMT(Server, "Server created on port {} with web root: {}", m_port, web_root.empty() ? "none" : web_root);
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
            LOG_WARN(Server, "Server is already running");
            return;
        }
        LOG_INFO_FMT(Server, "Starting server on port {}", m_port);

        RegisterInternalServices();

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
        LOG_INFO(Server, "Stopping server...");

        m_running.store(false);

        if (m_socket_server)
        {
            m_socket_server->Stop();
        }

        if (m_server_thread.joinable())
        {
            m_server_thread.join();
        }

        LOG_INFO(Server, "Server stopped");
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
            LOG_WARN_FMT(Server, "Cannot register service '{}': server is running", service_name);
            return false;
        }

        if (service_name.empty())
        {
            LOG_WARN(Server, "Cannot register service with empty name");
            return false;
        }

        if (!handler)
        {
            LOG_WARN_FMT(Server, "Cannot register service '{}': handler is null", service_name);
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
            LOG_INFO_FMT(Server, "Service '{}' registered successfully", service_name);
        }
        else 
        {
            LOG_WARN_FMT(Server, "Failed to register service '{}': name already exists", service_name);
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
            LOG_WARN_FMT(Server, "Cannot unregister service '{}': server is running", service_name);
            return false;
        }

        bool success = m_service_registry->UnregisterService(service_name);
        if (success) 
        {
            LOG_INFO_FMT(Server, "Service '{}' unregistered successfully", service_name);
        }
        else 
        {
            LOG_WARN_FMT(Server, "Failed to unregister service '{}': not found", service_name);
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
                LOG_ERROR_FMT(Server, "Failed to start server on port {}", m_port);
                m_running.store(false);
                return;
            }
            LOG_INFO_FMT(Server, "Server running on http://localhost:{}", m_port);

            // Run the server with our request handler
            m_socket_server->Run([this](const std::string& request_data) -> std::string
            {
                return HandleRequest(request_data);
            });
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_FMT(Server, "Server error: {}", e.what());
            m_running.store(false);
        }   
    }

    /**
     * @brief Register built-in internal HTTP services used by the server
     *
     */
    void Server::RegisterInternalServices()
    {
        // Health check endpoint
        RegisterService("ping", [this](const http::Request& request) -> http::Response 
        {
            (void)request; // Suppress unused parameter warning
            
            http::Response response;
            response.status = http::StatusCode::OK;
            
            // Create health check JSON response  
            std::ostringstream json;
            json << "{"
                 << "\"status\":\"ok\","
                 << "\"message\":\"ping\","
                 << "\"timestamp\":\"" << GetCurrentTimestamp() << "\","
                 << "\"services\":" << m_service_registry->GetServiceNames().size()
                 << "}";
            
            response.SetJson(json.str());
            return response;
        });

        // Hot reload status endpoint
        RegisterService("api/hotreload/status", [this](const http::Request& request) -> http::Response 
        {
            (void)request; // Suppress unused parameter warning
            
            http::Response response;
            response.status = http::StatusCode::OK;
            
            // Create hot reload status JSON response
            std::ostringstream json;
            json << "{"
                 << "\"isRunning\":false,"
                 << "\"loadedScriptsCount\":0,"
                 << "\"scriptDirectory\":\"./scripts\","
                 << "\"dotnetPath\":\"/usr/bin/dotnet\","
                 << "\"lastUpdate\":\"" << GetCurrentTimestamp() << "\","
                 << "\"supportedExtensions\":[\".cs\",\".dll\"]"
                 << "}";
            
            response.SetJson(json.str());
            return response;
        });

        // Server statistics endpoint
        RegisterService("api/server/stats", [this](const http::Request& request) -> http::Response 
        {
            (void)request; // Suppress unused parameter warning
            
            http::Response response;
            response.status = http::StatusCode::OK;
            
            // Get basic server statistics
            static auto start_time = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            auto uptime_seconds = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
            
            // Create server stats JSON response
            std::ostringstream json;
            json << "{"
                 << "\"uptime\":" << uptime_seconds << ","
                 << "\"uptimeFormatted\":\"" << FormatUptime(uptime_seconds) << "\","
                 << "\"requestCount\":0,"
                 << "\"memoryUsage\":\"N/A\","
                 << "\"port\":8080,"
                 << "\"version\":\"1.0.0\","
                 << "\"timestamp\":\"" << GetCurrentTimestamp() << "\""
                 << "}";
            
            response.SetJson(json.str());
            return response;
        });
    }

    /**
     * @brief Get current timestamp in ISO 8601 format
     * @return Current timestamp string
     */
    std::string Server::GetCurrentTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::ostringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
        
        return ss.str();
    }

    /**
     * @brief Format uptime seconds into human readable string
     * @param seconds Uptime in seconds
     * @return Formatted uptime string
     */
    std::string Server::FormatUptime(long seconds)
    {
        long days = seconds / 86400;
        long hours = (seconds % 86400) / 3600;
        long minutes = (seconds % 3600) / 60;
        
        std::ostringstream ss;
        if (days > 0) {
            ss << days << "天 " << hours << "小时 " << minutes << "分钟";
        } else if (hours > 0) {
            ss << hours << "小时 " << minutes << "分钟";
        } else {
            ss << minutes << "分钟";
        }
        
        return ss.str();
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
            LOG_DEBUG_FMT(Server, "Received request: {} bytes", request_data.size());
            
            // Parse HTTP request
            auto request_opt = http::HttpParser::ParseRequest(request_data);
            if (!request_opt)
            {
                LOG_WARN(Server, "Received invalid HTTP request");
                http::Response error_response;
                error_response.status = http::StatusCode::BadRequest;
                error_response.SetText("Bad Request");
                return http::HttpParser::SerializeResponse(error_response);
            }
            
            const auto& request = *request_opt;
            LOG_DEBUG_FMT(Server, 
                "Processing {} request to {}",
                http::MethodToString(request.method), request.path);
            
            // Use RequestRouter to handle the request
            http::Response response = m_request_router->RouteRequest(request);
            
            // Return serialized response
            return http::HttpParser::SerializeResponse(response);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_FMT(Server, "Error handling request: {}", e.what());
            http::Response error_response;
            error_response.status = http::StatusCode::InternalServerError;
            error_response.SetJson("{\"error\":\"Internal Server Error\"}");
            return http::HttpParser::SerializeResponse(error_response);
        }
    }

} // namespace core

