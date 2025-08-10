/**
 * @file logger.hpp
 * @brief Logger System Class Definition
 * @author Mini Server Team
 * @version 1.0.0
 * @date 2024
 */

#pragma once

#include <string>
#include <mutex>
#include <fstream>
#include <sstream>

namespace miniserver::utils
{
    // Log level enumeration
    enum class LogLevel {
        Debug = 0,   ///< Debug information
        Info = 1,    ///< General information
        Warning = 2, ///< Warning information
        Error = 3    ///< Error information
    };

    using LogLevelAlias = LogLevel; // Alias for conciseness
    static constexpr const char* kLoggerComponent = "Logger";

    /**
     * @brief Logger system (Singleton pattern)
     *
     * Provides thread-safe logging functionality, supports console and file output,
     * color output, and multiple log levels.
     */
    class Logger
    {
    public:
        using Level = LogLevel;
        /**
         * @brief Get singleton instance
         * @return Reference to Logger singleton
         */
        static Logger& GetInstance();
        /**
         * @brief Set minimum log level for output
         * @param level Minimum log level
         */
        void SetLogLevel(LogLevel level);
        /**
         * @brief Enable or disable console output
         * @param enable True to enable, false to disable
         */
        void EnableConsoleOutput(bool enable);
        /**
         * @brief Enable file output
         * @param filename Log file name, empty to disable file output
         */
        void EnableFileOutput(const std::string& filename = "");
        /**
         * @brief Enable or disable color output
         * @param enable True to enable, false to disable
         */
        void EnableColors(bool enable);
        /**
         * @brief Log a debug message
         * @param component Component name
         * @param message Log message
         */
        void Debug(const std::string& component, const std::string& message);
        /**
         * @brief Log an info message
         * @param component Component name
         * @param message Log message
         */
        void Info(const std::string& component, const std::string& message);
        /**
         * @brief Log a warning message
         * @param component Component name
         * @param message Log message
         */
        void Warning(const std::string& component, const std::string& message);
        /**
         * @brief Log an error message
         * @param component Component name
         * @param message Log message
         */
        void Error(const std::string& component, const std::string& message);
        /**
         * @brief Get current log level
         * @return Current log level
         */
        LogLevel GetLogLevel() const;
        /**
         * @brief Check if console output is enabled
         * @return True if enabled
         */
        bool IsConsoleOutputEnabled() const;
        /**
         * @brief Check if file output is enabled
         * @return True if enabled
         */
        bool IsFileOutputEnabled() const;
        /**
         * @brief Check if color output is enabled
         * @return True if enabled
         */
        bool IsColorsEnabled() const;
    private:
        /**
         * @brief Private constructor (singleton)
         */
        Logger();
        /**
         * @brief Private destructor
         */
        ~Logger();
        // Disable copy and move
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(Logger&&) = delete;
        /**
         * @brief Get current timestamp string
         * @return Formatted timestamp string
         */
        std::string GetCurrentTimestamp() const;
        /**
         * @brief Convert log level to string
         * @param level Log level
         * @return Log level string
         */
        std::string LevelToString(LogLevel level) const;
        /**
         * @brief Format log message
         * @param timestamp Timestamp
         * @param level Log level
         * @param component Component name
         * @param message Raw message
         * @return Formatted message string
         */
        std::string FormatMessage(const std::string& timestamp,
                                   LogLevel level,
                                   const std::string& component,
                                   const std::string& message) const;
        /**
         * @brief Get ANSI color code for log level
         * @param level Log level
         * @return ANSI color code string
         */
        std::string GetColorCode(LogLevel level) const;
        /**
         * @brief Get ANSI reset code
         * @return ANSI reset code string
         */
        std::string GetResetCode() const;
        /**
         * @brief Core log function (private)
         * @param level Log level
         * @param component Component name
         * @param message Log message
         */
        void Log(LogLevel level, const std::string& component, const std::string& message);

    private:
        mutable std::mutex m_log_mutex;      ///< Mutex for thread-safe logging
        LogLevel m_current_level;            ///< Current log level
        bool m_enable_console_output;        ///< Console output enabled
        bool m_enable_file_output;           ///< File output enabled
        bool m_enable_colors;                ///< Color output enabled
        std::ofstream m_log_file;            ///< Log file stream
    };
    /**
     * @brief Convenience function: log debug message
     * @param component Component name
     * @param message Log message
     */
    void LogDebug(const std::string& component, const std::string& message);
    /**
     * @brief Convenience function: log info message
     * @param component Component name
     * @param message Log message
     */
    void LogInfo(const std::string& component, const std::string& message);
    /**
     * @brief Convenience function: log warning message
     * @param component Component name
     * @param message Log message
     */
    void LogWarning(const std::string& component, const std::string& message);
    /**
     * @brief Convenience function: log error message
     * @param component Component name
     * @param message Log message
     */
    void LogError(const std::string& component, const std::string& message);
    // Simple placeholder formatting implementation (replace "{}" with arguments in order)
    namespace detail
    {
        inline void format_inplace(std::string&) {}
        template<typename T, typename... Rest>
        void format_inplace(std::string& fmt, T&& v, Rest&&... rest)
        {
            auto pos = fmt.find("{}");
            if (pos != std::string::npos)
            {
                std::ostringstream oss; oss << std::forward<T>(v);
                fmt.replace(pos, 2, oss.str());
            }
            if constexpr (sizeof...(rest) > 0) format_inplace(fmt, std::forward<Rest>(rest)...);
        }
        template<typename... Args>
        std::string format(std::string fmt, Args&&... args)
        {
            format_inplace(fmt, std::forward<Args>(args)...); return fmt;
        }
    }

    // Logging macros (component name fixed or passed)
#define MS_LOGGER (::miniserver::utils::Logger::GetInstance())
    #define LOG_DEBUG(comp, msg) MS_LOGGER.Debug(comp, msg)
    #define LOG_INFO(comp, msg)  MS_LOGGER.Info(comp, msg)
    #define LOG_WARN(comp, msg)  MS_LOGGER.Warning(comp, msg)
    #define LOG_ERROR(comp, msg) MS_LOGGER.Error(comp, msg)
    #define LOG_DEBUG_FMT(comp, fmt, ...) MS_LOGGER.Debug(comp, ::miniserver::utils::detail::format(fmt, __VA_ARGS__))
    #define LOG_INFO_FMT(comp, fmt, ...)  MS_LOGGER.Info(comp,  ::miniserver::utils::detail::format(fmt, __VA_ARGS__))
    #define LOG_WARN_FMT(comp, fmt, ...)  MS_LOGGER.Warning(comp,::miniserver::utils::detail::format(fmt, __VA_ARGS__))
    #define LOG_ERROR_FMT(comp, fmt, ...) MS_LOGGER.Error(comp, ::miniserver::utils::detail::format(fmt, __VA_ARGS__))
    // Legacy usage compatibility (default component is "Server")
    #define LOG_DEBUG_DEFAULT(fmt, ...) LOG_DEBUG_FMT("Server", fmt, __VA_ARGS__)
    #define LOG_INFO_DEFAULT(fmt, ...)  LOG_INFO_FMT("Server",  fmt, __VA_ARGS__)
    #define LOG_WARN_DEFAULT(fmt, ...)  LOG_WARN_FMT("Server",  fmt, __VA_ARGS__)
    #define LOG_ERROR_DEFAULT(fmt, ...) LOG_ERROR_FMT("Server", fmt, __VA_ARGS__)
} // namespace miniserver::utils

