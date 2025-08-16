/**
 * @file request_router.cpp
 * @brief HTTP Request Router Implementation
 * @author Mini Server Team
 * @version 1.0.0
 * @date 2024
 */

#include "request_router.hpp"
#include "service_registry.hpp"
#include "static_file_handler.hpp"
#include "utils/logger.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <filesystem>

namespace miniserver::core
{

    /**
     * @brief Constructor
     * @param serviceRegistry Pointer to ServiceRegistry
     * @param webRoot Web root directory for static files
     */
    RequestRouter::RequestRouter(services::ServiceRegistry* serviceRegistry, const std::string& webRoot)
        : m_service_registry(serviceRegistry)
    {
        if (!m_service_registry)
        {
            throw std::invalid_argument("ServiceRegistry cannot be null");
        }
        
        // Initialize static file handler if web root is provided
        if (!webRoot.empty() && std::filesystem::exists(webRoot))
        {
            m_static_file_handler = std::make_unique<StaticFileHandler>(webRoot);
            LOG_INFO_FMT("RequestRouter", "Static file handler initialized with root: {}", webRoot);
        }
        else if (!webRoot.empty())
        {
            LOG_WARN_FMT("RequestRouter", "Web root directory does not exist: {}", webRoot);
        }
        
        LOG_INFO("RequestRouter", "RequestRouter initialized");
    }

    /**
     * @brief Route HTTP request to appropriate handler
     * @param request HTTP request
     * @return HTTP response
     */
    http::Response RequestRouter::RouteRequest(const http::Request& request)
    {
        LOG_DEBUG_FMT("RequestRouter", "Routing {} request to {}",
                     http::MethodToString(request.method), request.path);

        http::Response response;

        try
        {
            response = RouteRequestInternal(request);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_FMT("RequestRouter", "Error routing request: {}", e.what());
            response = CreateErrorResponse(http::StatusCode::InternalServerError, "Internal Server Error");
        }

        // Add CORS headers to all responses
        AddCorsHeaders(response);

        LOG_DEBUG_FMT("RequestRouter", "Request routed successfully, response status: {}",
                     static_cast<int>(response.status));

        return response;
    }

    /**
     * @brief Internal routing logic without exception handling
     * @param request HTTP request
     * @return HTTP response
     */
    http::Response RequestRouter::RouteRequestInternal(const http::Request& request)
    {
        // 1. Handle CORS preflight requests (highest priority)
        if (request.method == http::Method::OPTIONS)
        {
            return HandleOptionsRequest(request);
        }

        // 2. Handle API endpoints
        if (request.method == http::Method::GET)
        {
            return HandleGetRequest(request);
        }
        else if (request.method == http::Method::POST)
        {
            return HandlePostRequest(request);
        }

        // 3. Method not allowed for other HTTP methods
        return CreateErrorResponse(http::StatusCode::MethodNotAllowed, "Method not allowed");
    }

    /**
     * @brief Handle GET requests
     * @param request HTTP request
     * @return HTTP response
     */
    http::Response RequestRouter::HandleGetRequest(const http::Request& request)
    {
        const std::string& path = request.path;

        // First check if it's a registered service (remove leading slash)
        std::string service_name = path.substr(1); // Remove leading "/"
        if (!service_name.empty() && m_service_registry->HasService(service_name))
        {
            return m_service_registry->HandleServiceRequest(request, service_name);
        }

        // Check for legacy /services endpoint
        if (path == "/services")
        {
            return m_service_registry->GetServicesInfo();
        }

        // Static file serving (if available)
        if (m_static_file_handler)
        {
            return m_static_file_handler->HandleRequest(request);
        }

        // Fallback for root endpoint (if no static file handler)
        if (path == "/")
        {
            return HandleRootRequest(request);
        }

        // Resource not found
        return CreateErrorResponse(http::StatusCode::NotFound, "Resource not found");
    }

    /**
     * @brief Handle POST requests
     * @param request HTTP request
     * @return HTTP response
     */
    http::Response RequestRouter::HandlePostRequest(const http::Request& request)
    {
        const std::string& path = request.path;

        // Service invocation endpoint
        if (path.length() > 9 && path.substr(0, 9) == "/service/")
        {
            std::string service_name = ExtractServiceName(path);
            if (service_name.empty())
            {
                return CreateErrorResponse(http::StatusCode::BadRequest, "Service name is required");
            }
            return m_service_registry->HandleServiceRequest(request, service_name);
        }

        // No POST endpoints match
        return CreateErrorResponse(http::StatusCode::NotFound, "Endpoint not found");
    }

    /**
     * @brief Add CORS headers to response
     * @param response HTTP response to modify
     */
    void RequestRouter::AddCorsHeaders(http::Response& response)
    {
        response.headers["Access-Control-Allow-Origin"] = "*";
        response.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
        response.headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
    }

    /**
     * @brief Handle OPTIONS preflight requests
     * @param request HTTP request
     * @return HTTP response
     */
    http::Response RequestRouter::HandleOptionsRequest(const http::Request& request)
    {
        (void)request; // Suppress unused parameter warning

        http::Response response;
        response.status = http::StatusCode::OK;
        response.headers["Access-Control-Allow-Origin"] = "*";
        response.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
        response.headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
        response.headers["Access-Control-Max-Age"] = "86400";
        response.body = "";

        LOG_DEBUG("RequestRouter", "Handled OPTIONS preflight request");
        return response;
    }

    /**
     * @brief Handle root requests (GET /)
     * @param request HTTP request
     * @return HTTP response
     */
    http::Response RequestRouter::HandleRootRequest(const http::Request& request)
    {
        (void)request; // Suppress unused parameter warning

        http::Response response;
        response.status = http::StatusCode::OK;
        
        // Create welcome JSON response with API information
        std::ostringstream json;
        json << "{"
             << "\"message\":\"Welcome to Mini Server\","
             << "\"version\":\"1.0.0\","
             << "\"endpoints\":{"
             << "\"health\":\"GET /ping\","
             << "\"services\":\"GET /services\","
             << "\"invoke\":\"POST /service/<name>\""
             << "},"
             << "\"timestamp\":\"" << GetCurrentTimestamp() << "\""
             << "}";
        
        response.SetJson(json.str());

        LOG_DEBUG("RequestRouter", "Handled root request");
        return response;
    }

    /**
     * @brief Create error response
     * @param status HTTP status code
     * @param message Error message
     * @return HTTP response
     */
    http::Response RequestRouter::CreateErrorResponse(http::StatusCode status, const std::string& message)
    {
        http::Response response;
        response.status = status;
        
        std::ostringstream json;
        json << "{"
             << "\"error\":\"" << message << "\","
             << "\"status\":" << static_cast<int>(status) << ","
             << "\"timestamp\":\"" << GetCurrentTimestamp() << "\""
             << "}";
        
        response.SetJson(json.str());

        LOG_DEBUG_FMT("RequestRouter", "Created error response: {} - {}", static_cast<int>(status), message);
        return response;
    }

    /**
     * @brief Extract service name from path
     * @param path Request path (e.g. /service/echo)
     * @return Service name, empty string if extraction fails
     */
    std::string RequestRouter::ExtractServiceName(const std::string& path)
    {
        const std::string service_prefix = "/service/";
        if (path.length() <= service_prefix.length())
        {
            return "";
        }
        
        std::string service_name = path.substr(service_prefix.length());
        
        // Remove any query parameters or fragments
        size_t query_pos = service_name.find('?');
        if (query_pos != std::string::npos)
        {
            service_name = service_name.substr(0, query_pos);
        }
        
        size_t fragment_pos = service_name.find('#');
        if (fragment_pos != std::string::npos)
        {
            service_name = service_name.substr(0, fragment_pos);
        }
        
        return service_name;
    }

    /**
     * @brief Get current timestamp in ISO 8601 format
     * @return Current timestamp string
     */
    std::string RequestRouter::GetCurrentTimestamp()
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

} // namespace core
