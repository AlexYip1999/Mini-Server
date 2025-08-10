/**
 * @file logger.cpp
 * @brief Logger System Implementation
 * @author Mini Server Team
 * @version 1.0.0
 * @date 2024
 */

#include "logger.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>

#ifdef _WIN32
    #include <windows.h>
    #undef FormatMessage  // Avoid Windows API conflict
#endif

namespace miniserver::utils
{

    Logger& Logger::GetInstance()
    {
        static Logger instance;
        return instance;
    }

    Logger::Logger()
        : m_current_level(LogLevel::Info)
        , m_enable_console_output(true)
        , m_enable_file_output(false)
        , m_enable_colors(true)
    {
    #ifdef _WIN32
        // Enable ANSI color support on Windows
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE)
        {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode))
            {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
            }
        }
    #endif
    }

    Logger::~Logger()
    {
        std::lock_guard<std::mutex> lock(m_log_mutex);
        if (m_log_file.is_open())
        {
            m_log_file.close();
        }
    }

    void Logger::SetLogLevel(LogLevel level)
    {
        std::lock_guard<std::mutex> lock(m_log_mutex);
        m_current_level = level;
    }

    void Logger::EnableConsoleOutput(bool enable)
    {
        std::lock_guard<std::mutex> lock(m_log_mutex);
        m_enable_console_output = enable;
    }

    void Logger::EnableFileOutput(const std::string& filename)
    {
        std::lock_guard<std::mutex> lock(m_log_mutex);
        if (m_log_file.is_open())
        {
            m_log_file.close();
        }
        if (!filename.empty())
        {
            m_log_file.open(filename, std::ios::app);
            m_enable_file_output = m_log_file.is_open();
            if (m_enable_file_output)
            {
                m_log_file << "\n=== Log session started " << GetCurrentTimestamp() << " ===\n";
                m_log_file.flush();
            }
        }
        else
        {
            m_enable_file_output = false;
        }
    }

    void Logger::EnableColors(bool enable)
    {
        std::lock_guard<std::mutex> lock(m_log_mutex);
        m_enable_colors = enable;
    }

    void Logger::Debug(const std::string& component, const std::string& message)
    {
        Log(LogLevel::Debug, component, message);
    }

    void Logger::Info(const std::string& component, const std::string& message)
    {
        Log(LogLevel::Info, component, message);
    }

    void Logger::Warning(const std::string& component, const std::string& message)
    {
        Log(LogLevel::Warning, component, message);
    }

    void Logger::Error(const std::string& component, const std::string& message)
    {
        Log(LogLevel::Error, component, message);
    }

    LogLevel Logger::GetLogLevel() const
    {
        std::lock_guard<std::mutex> lock(m_log_mutex);
        return m_current_level;
    }

    bool Logger::IsConsoleOutputEnabled() const
    {
        std::lock_guard<std::mutex> lock(m_log_mutex);
        return m_enable_console_output;
    }

    bool Logger::IsFileOutputEnabled() const
    {
        std::lock_guard<std::mutex> lock(m_log_mutex);
        return m_enable_file_output;
    }

    bool Logger::IsColorsEnabled() const
    {
        std::lock_guard<std::mutex> lock(m_log_mutex);
        return m_enable_colors;
    }

    void Logger::Log(LogLevel level,
                    const std::string& component,
                    const std::string& message)
    {
        std::lock_guard<std::mutex> lock(m_log_mutex);
        if (level < m_current_level)
        {
            return;
        }
        
        std::string timestamp = GetCurrentTimestamp();
        std::string level_str = LevelToString(level);
        std::string formatted_message = FormatMessage(timestamp, level, component, message);
        
        // Console output
        if (m_enable_console_output)
        {
        if (m_enable_colors)
            {
                std::cout << GetColorCode(level) << formatted_message << GetResetCode() << std::endl;
            }
            else
            {
                std::cout << formatted_message << std::endl;
            }
        }
        
        // File output
        if (m_enable_file_output && m_log_file.is_open())
        {
            m_log_file << formatted_message << std::endl;
            m_log_file.flush();
        }
    }

    std::string Logger::GetCurrentTimestamp() const
    {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    std::string Logger::LevelToString(LogLevel level) const
    {
        switch (level)
        {
            case LogLevel::Debug:   return "DEBUG";
            case LogLevel::Info:    return "INFO";
            case LogLevel::Warning: return "WARN";
            case LogLevel::Error:   return "ERROR";
            default:                return "UNKNOWN";
        }
    }

    std::string Logger::FormatMessage(const std::string& timestamp,
                                       LogLevel level,
                                       const std::string& component,
                                       const std::string& message) const
    {
        std::stringstream ss;
        ss << "[" << timestamp << "] "
           << "[" << LevelToString(level) << "] "
           << "[" << component << "] "
           << message;
        return ss.str();
    }

    std::string Logger::GetColorCode(LogLevel level) const
    {
        switch (level)
        {
            case LogLevel::Debug:   return "\033[36m"; // Cyan
            case LogLevel::Info:    return "\033[32m"; // Green
            case LogLevel::Warning: return "\033[33m"; // Yellow
            case LogLevel::Error:   return "\033[31m"; // Red
            default:                return "";
        }
    }

    std::string Logger::GetResetCode() const
    {
        return "\033[0m";
    }

    // Convenience functions
    void LogDebug(const std::string& component, const std::string& message)
    {
        Logger::GetInstance().Debug(component, message);
    }

    void LogInfo(const std::string& component, const std::string& message)
    {
        Logger::GetInstance().Info(component, message);
    }

    void LogWarning(const std::string& component, const std::string& message)
    {
        Logger::GetInstance().Warning(component, message);
    }

    void LogError(const std::string& component, const std::string& message)
    {
        Logger::GetInstance().Error(component, message);
    }

} // namespace miniserver::utils

