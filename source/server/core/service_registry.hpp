/**
 * @file service_registry.hpp
 * @brief Service Registry Class Definition
 * @author Mini Server Team
 * @version 1.0.0
 * @date 2024
 */

#pragma once

#include "../net/http_types.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <shared_mutex>
#include <optional>

namespace miniserver::services
{
    /**
     * @brief Service handler function type
     * @param request HTTP request object
     * @return HTTP response object
     */
    using ServiceHandler = std::function<http::Response(const http::Request&)>;
    /**
     * @brief Service information structure
     */
    struct ServiceInfo
    {
        std::string description;    ///< Service description
        std::string version;        ///< Service version
        ServiceHandler handler;     ///< Service handler function
        bool enabled = true;        ///< Whether service is enabled
        ServiceInfo() = default;
        ServiceInfo(const std::string& desc,
                   const std::string& ver,
                   ServiceHandler h,
                   bool enable = true)
            : description(desc), version(ver), handler(std::move(h)), enabled(enable) {}
    };
    /**
     * @brief Service Registry (Singleton Pattern)
     *
     * Manages all registered services, provides thread-safe registration,
     * deregistration and lookup functionality.
     * Uses read-write locks to optimize concurrent performance,
     * supports service enable/disable state control.
     */
    class ServiceRegistry
    {
    public:
        ServiceRegistry();
        ~ServiceRegistry();

        ServiceRegistry(const ServiceRegistry&) = delete;
        ServiceRegistry& operator=(const ServiceRegistry&) = delete;
        ServiceRegistry(ServiceRegistry&&) = delete;
        ServiceRegistry& operator=(ServiceRegistry&&) = delete;

        /**
         * @brief Register service
         * @param name Service name (must be unique)
         * @param info Service information
         * @return true if registration successful
         */
        bool RegisterService(const std::string& name, const ServiceInfo& info);
        /**
         * @brief Unregister service
         * @param name Service name to unregister
         * @return true if unregistration successful
         */
        bool UnregisterService(const std::string& name);
        /**
         * @brief Get service information
         * @param name Service name
         * @return Service information, or nullopt if not found
         */
        std::optional<ServiceInfo> GetService(const std::string& name) const;
        /**
         * @brief Get all service names
         * @return Vector containing all service names
         */
        std::vector<std::string> GetServiceNames() const;
        /**
         * @brief Get service count
         * @return Number of registered services
         */
        size_t GetServiceCount() const;
        /**
         * @brief Check if service exists
         * @param name Service name
         * @return true if service exists
         */
        bool HasService(const std::string& name) const;
        /**
         * @brief Clear all services
         */
        void ClearServices();
        /**
         * @brief Handle service request
         * @param request HTTP request object
         * @param serviceName Service name to handle
         * @return HTTP response
         */
        http::Response HandleServiceRequest(const http::Request& request,
                                           const std::string& serviceName);
        /**
         * @brief Get all services information (JSON format)
         * @return HTTP response containing all services information
         */
        http::Response GetServicesInfo() const;
        /**
         * @brief Enable service
         * @param name Service name
         * @return true if successful
         */
        bool EnableService(const std::string& name);
        /**
         * @brief Disable service
         * @param name Service name
         * @return true if successful
         */
        bool DisableService(const std::string& name);
    private:
        /**
         * @brief Create error response
         * @param status HTTP status code
         * @param message Error message
         * @return HTTP error response
         */
        static http::Response CreateErrorResponse(http::StatusCode status,
                                                  const std::string& message);
    private:
    mutable std::shared_mutex m_servicesMutex;                     ///< Read-write lock protecting service map
    std::unordered_map<std::string, ServiceInfo> m_services;       ///< Service map
    };
} // namespace miniserver::services

