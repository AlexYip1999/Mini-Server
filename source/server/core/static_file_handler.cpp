/**
 * @file static_file_handler.cpp
 * @brief Implementation of Static File Handler
 * @author Mini Server Team
 * @version 1.0.0
 */

#include "static_file_handler.hpp"
#include "../utils/logger.hpp"
#include <fstream>
#include <filesystem>
#include <sstream>

namespace miniserver::core
{

StaticFileHandler::StaticFileHandler(const std::string& root_directory)
    : m_root_directory(root_directory)
{
    // Initialize MIME types
    m_mime_types[".html"] = "text/html";
    m_mime_types[".htm"] = "text/html";
    m_mime_types[".css"] = "text/css";
    m_mime_types[".js"] = "application/javascript";
    m_mime_types[".json"] = "application/json";
    m_mime_types[".png"] = "image/png";
    m_mime_types[".jpg"] = "image/jpeg";
    m_mime_types[".jpeg"] = "image/jpeg";
    m_mime_types[".gif"] = "image/gif";
    m_mime_types[".svg"] = "image/svg+xml";
    m_mime_types[".ico"] = "image/x-icon";
    m_mime_types[".txt"] = "text/plain";
    m_mime_types[".xml"] = "application/xml";
    
    LOG_INFO_FMT("StaticFileHandler", "Initialized with root directory: {}", m_root_directory);
}

http::Response StaticFileHandler::HandleRequest(const http::Request& request)
{
    http::Response response;
    
    try
    {
        std::string path = request.path;
        
        // Default to index.html for root path
        if (path == "/" || path.empty())
        {
            path = "/index.html";
        }
        
        // Security check
        if (!IsSafePath(path))
        {
            response.status = http::StatusCode::BadRequest;
            response.SetText("Invalid path");
            return response;
        }
        
        // Build full file path
        std::string full_path = m_root_directory + path;
        
        // Check if file exists
        if (!std::filesystem::exists(full_path))
        {
            response.status = http::StatusCode::NotFound;
            response.SetText("File not found");
            return response;
        }
        
        // Check if it's a regular file
        if (!std::filesystem::is_regular_file(full_path))
        {
            response.status = http::StatusCode::NotFound;
            response.SetText("Not a file");
            return response;
        }
        
        // Read file content
        std::string content = ReadFile(full_path);
        if (content.empty() && std::filesystem::file_size(full_path) > 0)
        {
            response.status = http::StatusCode::InternalServerError;
            response.SetText("Failed to read file");
            return response;
        }
        
        // Set response
        response.status = http::StatusCode::OK;
        response.body = content;
        
        // Set content type
        std::filesystem::path file_path(full_path);
        std::string extension = file_path.extension().string();
        std::string mime_type = GetMimeType(extension);
        response.SetHeader("Content-Type", mime_type);
        
        // Add CORS headers for API access
        response.AddCorsHeaders();
        
        LOG_DEBUG_FMT("StaticFileHandler", "Served file: {} ({})", path, mime_type);
        
    }
    catch (const std::exception& e)
    {
        LOG_ERROR_FMT("StaticFileHandler", "Error serving file {}: {}", request.path, e.what());
        response.status = http::StatusCode::InternalServerError;
        response.SetText("Internal server error");
    }
    
    return response;
}

void StaticFileHandler::SetRootDirectory(const std::string& root_directory)
{
    m_root_directory = root_directory;
    LOG_INFO_FMT("StaticFileHandler", "Root directory changed to: {}", m_root_directory);
}

std::string StaticFileHandler::GetMimeType(const std::string& extension) const
{
    auto it = m_mime_types.find(extension);
    if (it != m_mime_types.end())
    {
        return it->second;
    }
    return "application/octet-stream"; // Default MIME type
}

bool StaticFileHandler::IsSafePath(const std::string& path) const
{
    // Check for directory traversal attempts
    if (path.find("..") != std::string::npos)
    {
        return false;
    }
    
    // Path should start with /
    if (path.empty() || path[0] != '/')
    {
        return false;
    }
    
    // Check for null bytes
    if (path.find('\0') != std::string::npos)
    {
        return false;
    }
    
    return true;
}

std::string StaticFileHandler::ReadFile(const std::string& file_path) const
{
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open())
    {
        return "";
    }
    
    std::ostringstream content_stream;
    content_stream << file.rdbuf();
    return content_stream.str();
}

} // namespace miniserver::core
