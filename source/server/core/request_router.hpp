/**
 * @file request_router.hpp
 * @brief HTTP Request Router Class Definition
 * @author Mini Server Team
 * @version 1.0.0
 * @date 2024
 */

#pragma once

#include "net/http_types.hpp"
#include "static_file_handler.hpp"
#include <memory>

namespace miniserver::services
{
    class ServiceRegistry;
}

namespace miniserver::core
{
    /**
     * @brief RequestRouter routes HTTP requests to appropriate handlers
     */
    class RequestRouter
    {
    public:
        /**
         * @brief Constructor
         * @param serviceRegistry Pointer to ServiceRegistry
         * @param webRoot Web root directory for static files (optional)
         */
        RequestRouter(services::ServiceRegistry* serviceRegistry, const std::string& webRoot = "");
        /**
         * @brief Destructor
         */
        ~RequestRouter() = default;
        // Disable copy constructor and assignment
        RequestRouter(const RequestRouter&) = delete;
        RequestRouter& operator=(const RequestRouter&) = delete;
        /**
         * @brief Route HTTP request
         * @param request HTTP request
         * @return HTTP response
         */
        http::Response RouteRequest(const http::Request& request);

    private:
        services::ServiceRegistry* m_service_registry; ///< Service registry
        std::unique_ptr<StaticFileHandler> m_static_file_handler; ///< Static file handler
        /**
         * @brief Handle OPTIONS preflight requests
         * @param request HTTP request
         * @return HTTP response
         */
        http::Response HandleOptionsRequest(const http::Request& request);
        /**
         * @brief Handle root requests (GET /)
         * @param request HTTP request
         * @return HTTP response
         */
        http::Response HandleRootRequest(const http::Request& request);
        /**
         * @brief Create error response
         * @param status HTTP status code
         * @param message Error message
         * @return HTTP response
         */
        http::Response CreateErrorResponse(http::StatusCode status, const std::string& message);
        /**
         * @brief Extract service name from path
         * @param path Request path (e.g. /api/services/echo)
         * @return Service name, empty string if extraction fails
         */
        std::string ExtractServiceName(const std::string& path);
        /**
         * @brief Get current timestamp in ISO 8601 format
         * @return Current timestamp string
         */
        std::string GetCurrentTimestamp();
        /**
         * @brief Internal routing logic without exception handling
         * @param request HTTP request
         * @return HTTP response
         */
        http::Response RouteRequestInternal(const http::Request& request);
        /**
         * @brief Handle GET requests
         * @param request HTTP request
         * @return HTTP response
         */
        http::Response HandleGetRequest(const http::Request& request);
        /**
         * @brief Handle POST requests
         * @param request HTTP request
         * @return HTTP response
         */
        http::Response HandlePostRequest(const http::Request& request);
        /**
         * @brief Add CORS headers to response
         * @param response HTTP response to modify
         */
        void AddCorsHeaders(http::Response& response);
        /**
         * @brief Format uptime seconds into human readable string
         * @param seconds Uptime in seconds
         * @return Formatted uptime string
         */
        std::string FormatUptime(long seconds);
    };
} // namespace core

