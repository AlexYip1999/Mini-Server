/**
 * @file request_router.hpp
 * @brief HTTP Request Router Class Definition
 * @author Mini Server Team
 * @version 1.0.0
 * @date 2024
 */

#pragma once

#include "net/http_types.hpp"

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
         */
        explicit RequestRouter(services::ServiceRegistry* serviceRegistry);
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
        /**
         * @brief Handle OPTIONS preflight requests
         * @param request HTTP request
         * @return HTTP response
         */
        http::Response HandleOptionsRequest(const http::Request& request);
        /**
         * @brief Handle health check requests (GET /health)
         * @param request HTTP request
         * @return HTTP response
         */
        http::Response HandleHealthCheck(const http::Request& request);
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
    };
} // namespace core

