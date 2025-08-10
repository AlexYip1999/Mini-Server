/**
 * @file http_types.hpp
 * @brief HTTP related type definitions
 */

#pragma once

#include <string>
#include <map>
#include <vector>

namespace miniserver::http
{

/**
 * @brief HTTP method enumeration
 */
enum class Method {
    GET,
    POST,
    PUT,
    DELETE,
    OPTIONS,
    HEAD,
    PATCH,
    UNKNOWN
};

/**
 * @brief HTTP status code enumeration
 */
enum class StatusCode {
    OK = 200,
    Created = 201,
    NoContent = 204,
    BadRequest = 400,
    NotFound = 404,
    MethodNotAllowed = 405,
    InternalServerError = 500,
    NotImplemented = 501
};

/**
 * @brief HTTP request structure
 */
struct Request
{
    Method method = Method::UNKNOWN;                    ///< HTTP方法
    std::string path;                                   ///< 请求路径
    std::string query_string;                           ///< 查询字符串
    std::map<std::string, std::string> headers;         ///< 请求头（小写键名）
    std::string body;                                   ///< 请求体
    
    /**
     * @brief Check if the request is valid
     * @return true if valid, false otherwise
     */
    bool IsValid() const noexcept
    {
        return method != Method::UNKNOWN && !path.empty();
    }
    
    /**
     * @brief Get the value of a header (case-insensitive)
     * @param name Header name
     * @return Header value, or empty string if not found
     */
    std::string GetHeader(const std::string& name) const;
    
    /**
     * @brief Check if a header exists (case-insensitive)
     * @param name Header name
     * @return true if header exists, false otherwise
     */
    bool HasHeader(const std::string& name) const;
};

/**
 * @brief HTTP response structure
 */
struct Response
{
    StatusCode status = StatusCode::OK;                 ///< HTTP状态码
    std::map<std::string, std::string> headers;         ///< 响应头
    std::string body;                                   ///< 响应体
    
    /**
     * @brief 设置响应内容
     * @param content 响应内容
     * @param content_type 内容类型
     */
void SetContent(const std::string& content, const std::string& content_type);
    
    /**
     * @brief 设置JSON响应
     * @param json_content JSON内容
     */
    void SetJson(const std::string& json_content);
    
    /**
     * @brief 设置纯文本响应
     * @param text_content 文本内容
     */
    void SetText(const std::string& text_content);
    
    /**
     * @brief 设置响应头
     * @param name 头名称
     * @param value 头值
     */
    void SetHeader(const std::string& name, const std::string& value);
    
    /**
     * @brief 添加CORS头
     */
    void AddCorsHeaders();
};

/**
 * @brief Convert Method enum to string
 * @param method HTTP method
 * @return Method string
 */
std::string MethodToString(Method method);

/**
 * @brief Convert string to Method enum
 * @param method_str Method string
 * @return HTTP method enum
 */
Method StringToMethod(const std::string& method_str);

/**
 * @brief Convert StatusCode enum to string
 * @param status Status code
 * @return Status string
 */
std::string StatusToString(StatusCode status);

/**
 * @brief Get numeric value of status code
 * @param status Status code
 * @return Status code numeric value
 */
int StatusToInt(StatusCode status);

} // namespace miniserver::http

