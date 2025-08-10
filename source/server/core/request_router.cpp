/**
 * @file request_router.cpp
 * @brief HTTP Request Router Implementation
 * @author Mini Server Team
 * @version 1.0.0
 * @date 2024
 */

#include "request_router.hpp"
#include "service_registry.hpp"
#include "utils/logger.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>

namespace miniserver::core
{

    /**
     * @brief Constructor
     * @param serviceRegistry Pointer to ServiceRegistry
     */
    RequestRouter::RequestRouter(services::ServiceRegistry* serviceRegistry)
        : m_service_registry(serviceRegistry)
    {
        if (!m_service_registry)
        {
            throw std::invalid_argument("ServiceRegistry cannot be null");
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
            // Handle CORS preflight requests
            if (request.method == http::Method::OPTIONS)
            {
                response = HandleOptionsRequest(request);
            }
            // Health check endpoint
            else if (request.path == "/ping" && request.method == http::Method::GET)
            {
                response = HandleHealthCheck(request);
            }
            // Root endpoint
            else if (request.path == "/" && request.method == http::Method::GET)
            {
                response = HandleRootRequest(request);
            }
            // List services endpoint
            else if (request.path == "/services" && request.method == http::Method::GET)
            {
                response = m_service_registry->GetServicesInfo();
            }
            // Service invocation endpoint /service/<name>
            else if (request.path.substr(0, 9) == "/service/" && request.method == http::Method::POST)
            {
                std::string service_name = ExtractServiceName(request.path);
                if (!service_name.empty())
                {
                    response = m_service_registry->HandleServiceRequest(request, service_name);
                }
                else
                {
                    response = CreateErrorResponse(http::StatusCode::BadRequest, "Service name is required");
                }
            }
            // Default response for unknown endpoints
            else
            {
                response = CreateErrorResponse(http::StatusCode::NotFound, "Endpoint not found");
            }

            // Add CORS headers to all responses
            response.headers["Access-Control-Allow-Origin"] = "*";
            response.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
            response.headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";

            LOG_DEBUG_FMT("RequestRouter", "Request routed successfully, response status: {}",
                         static_cast<int>(response.status));
        }
        catch (const std::exception& e)
        {
            LOG_ERROR_FMT("RequestRouter", "Error routing request: {}", e.what());
            response = CreateErrorResponse(http::StatusCode::InternalServerError, "Internal Server Error");
        }

        return response;
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
     * @brief Handle health check requests (GET /ping)
     * @param request HTTP request
     * @return HTTP response
     */
    http::Response RequestRouter::HandleHealthCheck(const http::Request& request)
    {
        (void)request; // Suppress unused parameter warning

        http::Response response;
        response.status = http::StatusCode::OK;
        
        // Create health check JSON response
        std::ostringstream json;
        json << "{"
             << "\"status\":\"ok\","
             << "\"message\":\"pong\","
             << "\"timestamp\":\"" << GetCurrentTimestamp() << "\","
             << "\"services\":" << m_service_registry->GetServiceNames().size()
             << "}";
        
        response.SetJson(json.str());

        LOG_DEBUG("RequestRouter", "Handled health check request");
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
