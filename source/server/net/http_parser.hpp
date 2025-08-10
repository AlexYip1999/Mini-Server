/**
 * @file http_parser.hpp
 * @brief HTTP Request Parser
 * @author Mini Server Team
 * @version 1.0.0
 */

#pragma once

#include "http_types.hpp"
#include <string>
#include <optional>

namespace miniserver::http
{

/**
 * @brief HTTP Parser Class
 * 
 * Responsible for parsing raw HTTP request data and building Request objects
 */
class HttpParser {
public:
    /**
     * @brief Parse HTTP request
     * @param raw_data Raw HTTP request data
     * @return Parsed Request object, or nullopt if parsing fails
     */
    static std::optional<Request> ParseRequest(const std::string& raw_data);
    
    /**
     * @brief Build HTTP response string
     * @param response Response object
     * @return HTTP response string
     */
    static std::string SerializeResponse(const Response& response);

private:
    /**
     * @brief Parse request line
     * @param request_line Request line string
     * @param request Request object to fill
     * @return true if parsing succeeds
     */
    static bool ParseRequestLine(const std::string& request_line, Request& request);
    
    /**
     * @brief Parse request headers
     * @param header_line Header line string
     * @param request Request object to fill
     */
    static void ParseHeaderLine(const std::string& header_line, Request& request);
    
    /**
     * @brief Parse query parameters
     * @param url Original URL
     * @param request Request object to fill
     */
    static void ParseQueryParameters(const std::string& url, Request& request);
    
    /**
     * @brief Parse HTTP method from string
     * @param method_str Method string
     * @return HTTP method enum, or nullopt if invalid
     */
    static std::optional<Method> ParseMethod(const std::string& method_str);
    
    /**
     * @brief Parse HTTP version from string  
     * @param version_str Version string
     * @return HTTP version string, or nullopt if invalid
     */
    static std::optional<std::string> ParseVersion(const std::string& version_str);
    
    /**
     * @brief Trim whitespace from string
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string Trim(const std::string& str);
    
    /**
     * @brief URL decode string
     * @param str String to decode
     * @return Decoded string
     */
    static std::string UrlDecode(const std::string& str);
    
    /**
     * @brief Get status text for status code
     * @param status Status code
     * @return Status text
     */
    static std::string GetStatusText(StatusCode status);
};

} // namespace miniserver::http

