/**
 * @file http_types.cpp
 * @brief HTTP协议类型实现
 * @author Mini Server Team
 * @version 1.0.0
/**
 * @file http_types.cpp
 * @brief HTTP type definitions implementation
 * @author Mini Server Team
 * @version 1.0.0
 * @date 2024
 */
// Include definitions and utilities
#include "http_types.hpp"
#include <algorithm>
#include <cctype>
#include <string>

namespace miniserver::http
{

std::string Request::GetHeader(const std::string& name) const
{
    std::string lower_name = name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(),[](unsigned char c)
    {
        return static_cast<char>(std::tolower(c));
    });
        
    auto it = headers.find(lower_name);
    return (it != headers.end()) ? it->second : "";
}

bool Request::HasHeader(const std::string& name) const
{
    std::string lower_name = name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), [](unsigned char c)
    {
        return static_cast<char>(std::tolower(c));
    });
    
    return headers.find(lower_name) != headers.end();
}

void Response::SetContent(const std::string& content, const std::string& content_type)
{
    body = content;
    SetHeader("Content-Type", content_type);
    SetHeader("Content-Length", std::to_string(content.size()));
}

void Response::SetJson(const std::string& json_content)
{
    SetContent(json_content, "application/json; charset=utf-8");
}

void Response::SetText(const std::string& text_content)
{
    SetContent(text_content, "text/plain; charset=utf-8");
}

void Response::SetHeader(const std::string& name, const std::string& value)
{
    headers[name] = value;
}

void Response::AddCorsHeaders()
{
    SetHeader("Access-Control-Allow-Origin", "*");
    SetHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    SetHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    SetHeader("Access-Control-Max-Age", "3600");
}

std::string MethodToString(Method method)
{
    switch (method) {
        case Method::GET: return "GET";
        case Method::POST: return "POST";
        case Method::PUT: return "PUT";
        case Method::DELETE: return "DELETE";
        case Method::OPTIONS: return "OPTIONS";
        case Method::HEAD: return "HEAD";
        case Method::PATCH: return "PATCH";
        case Method::UNKNOWN:
        default: return "UNKNOWN";
    }
}

Method StringToMethod(const std::string& method_str)
{
    std::string upper_method = method_str;
    std::transform(upper_method.begin(), upper_method.end(), upper_method.begin(), [](unsigned char c)
    {
        return static_cast<char>(std::toupper(c));
    });
    
    if (upper_method == "GET") return Method::GET;
    if (upper_method == "POST") return Method::POST;
    if (upper_method == "PUT") return Method::PUT;
    if (upper_method == "DELETE") return Method::DELETE;
    if (upper_method == "OPTIONS") return Method::OPTIONS;
    if (upper_method == "HEAD") return Method::HEAD;
    if (upper_method == "PATCH") return Method::PATCH;
    
    return Method::UNKNOWN;
}

std::string StatusToString(StatusCode status)
{
    switch (status) {
        case StatusCode::OK: return "OK";
        case StatusCode::Created: return "Created";
        case StatusCode::NoContent: return "No Content";
        case StatusCode::BadRequest: return "Bad Request";
        case StatusCode::NotFound: return "Not Found";
        case StatusCode::MethodNotAllowed: return "Method Not Allowed";
        case StatusCode::InternalServerError: return "Internal Server Error";
        case StatusCode::NotImplemented: return "Not Implemented";
        default: return "Unknown";
    }
}

int StatusToInt(StatusCode status)
{
    return static_cast<int>(status);
}

} // namespace miniserver::http

