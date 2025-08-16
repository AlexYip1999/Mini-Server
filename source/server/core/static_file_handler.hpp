/**
 * @file static_file_handler.hpp
 * @brief Static File Handler for serving web content
 * @author Mini Server Team
 * @version 1.0.0
 */

#pragma once

#include "net/http_types.hpp"
#include <string>
#include <unordered_map>

namespace miniserver::core
{
    /**
     * @brief Static file handler for serving web content
     */
    class StaticFileHandler
    {
    public:
        /**
         * @brief Constructor
         * @param root_directory Root directory for static files
         */
        explicit StaticFileHandler(const std::string& root_directory);

        /**
         * @brief Handle static file request
         * @param request HTTP request
         * @return HTTP response
         */
        http::Response HandleRequest(const http::Request& request);

        /**
         * @brief Set the root directory for static files
         * @param root_directory New root directory
         */
        void SetRootDirectory(const std::string& root_directory);

    private:
        /**
         * @brief Get MIME type for file extension
         * @param extension File extension (with dot)
         * @return MIME type string
         */
        std::string GetMimeType(const std::string& extension) const;

        /**
         * @brief Check if path is safe (no directory traversal)
         * @param path Path to check
         * @return true if safe, false otherwise
         */
        bool IsSafePath(const std::string& path) const;

        /**
         * @brief Read file content
         * @param file_path Path to file
         * @return File content as string
         */
        std::string ReadFile(const std::string& file_path) const;

    private:
        std::string m_root_directory;                           ///< Root directory for static files
        std::unordered_map<std::string, std::string> m_mime_types; ///< MIME type mapping
    };

} // namespace miniserver::core
