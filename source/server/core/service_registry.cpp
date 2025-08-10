/**
 * @file service_registry.cpp
 * @brief Service Registry Implementation
 * @author Mini Server Team
 * @version 1.0.0
 * @date 2024
 */

#include "service_registry.hpp"
#include "utils/logger.hpp"

#include <algorithm>

namespace miniserver::services
{
    // Alias for HTTP namespace
    namespace http = miniserver::http;

    ServiceRegistry& ServiceRegistry::GetInstance()
    {
        static ServiceRegistry instance;
        return instance;
    }

    ServiceRegistry::ServiceRegistry()
    {
        LOG_INFO("ServiceRegistry", "Initialized");
    }

    ServiceRegistry::~ServiceRegistry()
    {
        std::lock_guard<std::shared_mutex> lock(m_servicesMutex);
            m_services.clear();
        LOG_INFO("ServiceRegistry", "Destroyed");
    }

    bool ServiceRegistry::RegisterService(const std::string& name, const ServiceInfo& info)
    {
        if (name.empty())
        {
            return false;
        }
        std::lock_guard<std::shared_mutex> lock(m_servicesMutex);
        if (m_services.find(name) != m_services.end())
        {
            LOG_WARN("ServiceRegistry", "Service already exists: " + name);
            return false;
        }
        m_services[name] = info;
        LOG_INFO("ServiceRegistry", "Registered service: " + name + " v" + info.version);
        return true;
    }

    bool ServiceRegistry::UnregisterService(const std::string& name)
    {
        if (name.empty())
        {
            return false;
        }

        std::lock_guard<std::shared_mutex> lock(m_servicesMutex);

        auto it = m_services.find(name);
        if (it != m_services.end())
        {
            m_services.erase(it);
            LOG_INFO("ServiceRegistry", "Unregistered service: " + name);
            return true;
        }
        LOG_WARN("ServiceRegistry", "Failed to unregister non-existent service: " + name);
        return false;
    }

    std::optional<ServiceInfo> ServiceRegistry::GetService(const std::string& name) const
    {
        if (name.empty())
        {
            return std::nullopt;
        }

        std::shared_lock<std::shared_mutex> lock(m_servicesMutex);

        auto it = m_services.find(name);
        if (it != m_services.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    std::vector<std::string> ServiceRegistry::GetServiceNames() const
    {
        std::shared_lock<std::shared_mutex> lock(m_servicesMutex);

        std::vector<std::string> names;
        names.reserve(m_services.size());
        for (const auto& [name, info] : m_services)
        {
            names.push_back(name);
        }
        return names;
    }

    size_t ServiceRegistry::GetServiceCount() const
    {
        std::shared_lock<std::shared_mutex> lock(m_servicesMutex);
        return m_services.size();
    }

    bool ServiceRegistry::HasService(const std::string& name) const
    {
        if (name.empty())
        {
            return false;
        }
        std::shared_lock<std::shared_mutex> lock(m_servicesMutex);
        return m_services.find(name) != m_services.end();
    }

    void ServiceRegistry::ClearServices()
    {
        std::lock_guard<std::shared_mutex> lock(m_servicesMutex);
        auto count = m_services.size();
        m_services.clear();
        LOG_INFO("ServiceRegistry", "Cleared " + std::to_string(count) + " services");
    }

    http::Response ServiceRegistry::HandleServiceRequest(
        const http::Request& request,
        const std::string& serviceName)
    {
        auto service_opt = GetService(serviceName);
        if (!service_opt)
        {
            LOG_WARN("ServiceRegistry", "Requested non-existent service: " + serviceName);
            return CreateErrorResponse(http::StatusCode::NotFound, "Service not found: " + serviceName);
        }
        const auto& service = *service_opt;
        if (!service.enabled)
        {
            LOG_WARN("ServiceRegistry", "Requested disabled service: " + serviceName);
            return CreateErrorResponse(http::StatusCode::InternalServerError, "Service disabled: " + serviceName);
        }
        try
        {
            LOG_DEBUG("ServiceRegistry", "Invoke service: " + serviceName);
            return service.handler(request);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("ServiceRegistry", "Exception in service '" + serviceName + "': " + e.what());
            return CreateErrorResponse(http::StatusCode::InternalServerError, "Internal service error");
        }
    }

    http::Response ServiceRegistry::GetServicesInfo() const
    {
    std::shared_lock<std::shared_mutex> lock(m_servicesMutex);
        std::string json = "{\n  \"services\": [\n";
        bool first = true;
    for (const auto& [name, info] : m_services)
        {
            if (!first)
            {
                json += ",\n";
            }
            first = false;
            json += "    {\n";
            json += "      \"name\": \"" + name + "\",\n";
            json += "      \"description\": \"" + info.description + "\",\n";
            json += "      \"version\": \"" + info.version + "\",\n";
            json += "      \"enabled\": " + std::string(info.enabled ? "true" : "false") + "\n";
            json += "    }";
        }
        json += "\n  ],\n";
        json += "  \"total\": " + std::to_string(m_services.size()) + "\n";
        json += "}";
        http::Response resp;
        resp.status = http::StatusCode::OK;
        resp.headers["Content-Type"] = "application/json";
        resp.headers["Cache-Control"] = "no-cache";
        resp.body = json;
        return resp;
    }

    bool ServiceRegistry::EnableService(const std::string& name)
    {
        if (name.empty())
        {
            return false;
        }
    std::lock_guard<std::shared_mutex> lock(m_servicesMutex);
    auto it = m_services.find(name);
        if (it != m_services.end())
        {
            it->second.enabled = true;
            LOG_INFO("ServiceRegistry", "Enabled service: " + name);
            return true;
        }
        LOG_WARN("ServiceRegistry", "Failed to enable non-existent service: " + name);
        return false;
    }

    bool ServiceRegistry::DisableService(const std::string& name)
    {
        if (name.empty())
        {
            return false;
        }
    std::lock_guard<std::shared_mutex> lock(m_servicesMutex);
    auto it = m_services.find(name);
        if (it != m_services.end())
        {
            it->second.enabled = false;
            LOG_INFO("ServiceRegistry", "Disabled service: " + name);
            return true;
        }
        LOG_WARN("ServiceRegistry", "Failed to disable non-existent service: " + name);
        return false;
    }

    http::Response ServiceRegistry::CreateErrorResponse(
        http::StatusCode status,
        const std::string& message)
    {
        http::Response resp;
        resp.status = status;
        resp.headers["Content-Type"] = "application/json";
        resp.body = "{\"error\": \"" + message + "\"}";
        return resp;
    }

} // namespace miniserver::services

