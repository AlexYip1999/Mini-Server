/**
 * @file http_parser.cpp
 * @brief HTTP Request Parser Implementation
 * @author Mini Server Team
 * @version 1.0.0
 */

#include "http_parser.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace miniserver::http
{

std::optional<Request> HttpParser::ParseRequest(const std::string& raw_data)
{
    if (raw_data.empty())
    {
        return std::nullopt;
    }
    
    Request request;
    std::istringstream stream(raw_data);
    std::string line;
    
    // Parse request line
    if (!std::getline(stream, line))
    {
        return std::nullopt;
    }
    
    // Remove \r if present
    if (!line.empty() && line.back() == '\r')
    {
        line.pop_back();
    }
    
    if (!ParseRequestLine(line, request))
    {
        return std::nullopt;
    }
    
    // Parse headers
    while (std::getline(stream, line))
    {
        // Remove \r if present
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        
        // Empty line marks end of headers
        if (line.empty())
        {
            break;
        }
        
        ParseHeaderLine(line, request);
    }
    
    // Parse body (rest of the stream)
    std::string body;
    std::string body_line;
    while (std::getline(stream, body_line))
    {
        if (!body.empty())
        {
            body += "\n";
        }
        body += body_line;
    }
    request.body = body;
    
    return request;
}

std::string HttpParser::SerializeResponse(const Response& response)
{
    std::ostringstream stream;
    
    // Status line
    stream << "HTTP/1.1 " << static_cast<int>(response.status) << " " 
           << GetStatusText(response.status) << "\r\n";
    
    // Headers
    for (const auto& [name, value] : response.headers)
    {
        stream << name << ": " << value << "\r\n";
    }
    
    // Content-Length header if not present
    if (response.headers.find("Content-Length") == response.headers.end())
    {
        stream << "Content-Length: " << response.body.size() << "\r\n";
    }
    
    // Empty line
    stream << "\r\n";
    
    // Body
    stream << response.body;
    
    return stream.str();
}

bool HttpParser::ParseRequestLine(const std::string& request_line, Request& request)
{
    std::istringstream stream(request_line);
    std::string method_str, url, version;
    
    if (!(stream >> method_str >> url >> version))
    {
        return false;
    }
    
    // Parse method
    auto method_opt = ParseMethod(method_str);
    if (!method_opt)
    {
        return false;
    }
    request.method = *method_opt;
    
    // Parse URL and query parameters
    ParseQueryParameters(url, request);
    
    return true;
}

void HttpParser::ParseHeaderLine(const std::string& header_line, Request& request)
{
    size_t colon_pos = header_line.find(':');
    if (colon_pos == std::string::npos)
    {
        return;
    }
    
    std::string name = Trim(header_line.substr(0, colon_pos));
    std::string value = Trim(header_line.substr(colon_pos + 1));
    
    // Convert header name to lowercase for case-insensitive lookup
    std::transform(name.begin(), name.end(), name.begin(),
                  [](unsigned char c) { return std::tolower(c); });
    
    request.headers[name] = value;
}

void HttpParser::ParseQueryParameters(const std::string& url, Request& request)
{
    size_t question_pos = url.find('?');
    if (question_pos == std::string::npos)
    {
        request.path = UrlDecode(url);
        request.query_string = "";
    }
    else
    {
        request.path = UrlDecode(url.substr(0, question_pos));
        request.query_string = url.substr(question_pos + 1);
    }
}

std::optional<Method> HttpParser::ParseMethod(const std::string& method_str)
{
    std::string upper_method = method_str;
    std::transform(upper_method.begin(), upper_method.end(), upper_method.begin(),
                  [](unsigned char c) { return std::toupper(c); });
    
    if (upper_method == "GET") return Method::GET;
    if (upper_method == "POST") return Method::POST;
    if (upper_method == "PUT") return Method::PUT;
    if (upper_method == "DELETE") return Method::DELETE;
    if (upper_method == "OPTIONS") return Method::OPTIONS;
    if (upper_method == "HEAD") return Method::HEAD;
    if (upper_method == "PATCH") return Method::PATCH;
    
    return std::nullopt;
}

std::optional<std::string> HttpParser::ParseVersion(const std::string& version_str)
{
    if (version_str == "HTTP/1.0" || version_str == "HTTP/1.1")
    {
        return version_str;
    }
    return std::nullopt;
}

std::string HttpParser::Trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        return "";
    }
    
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string HttpParser::UrlDecode(const std::string& str)
{
    std::string result;
    result.reserve(str.size());
    
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == '%' && i + 2 < str.size())
        {
            int hex_value;
            std::istringstream hex_stream(str.substr(i + 1, 2));
            if (hex_stream >> std::hex >> hex_value)
            {
                result += static_cast<char>(hex_value);
                i += 2;
            }
            else
            {
                result += str[i];
            }
        }
        else if (str[i] == '+')
        {
            result += ' ';
        }
        else
        {
            result += str[i];
        }
    }
    
    return result;
}

std::string HttpParser::GetStatusText(StatusCode status)
{
    switch (status)
    {
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

} // namespace miniserver::http
