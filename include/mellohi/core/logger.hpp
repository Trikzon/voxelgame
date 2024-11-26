#pragma once

#include <format>
#include <sstream>

#include "mellohi/core/types.hpp"

#define MH_LOG_WARN_ENABLED
#define MH_LOG_INFO_ENABLED
#define MH_LOG_DEBUG_ENABLED
#define MH_LOG_TRACE_ENABLED

#ifndef MH_DEBUG_MODE
    #undef MH_LOG_DEBUG_ENABLED
    #undef MH_LOG_TRACE_ENABLED
#endif

namespace mellohi
{
    enum class LogLevel : usize
    {
        Fatal = 0,
        Error = 1,
        Warn  = 2,
        Info  = 3,
        Debug = 4,
        Trace = 5,
    };

    template <typename... Args>
    void log(LogLevel level, const char *file_path, int line, const char *message, Args &&...args)
    {
        const char *tags[6] = {"FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"};
        
        const auto get_file_name = [](const char *file_path) -> const char *
        {
            const char *file_name = std::strrchr(file_path, '/');
            if (file_name == nullptr)
            {
                file_name = std::strchr(file_path, '\\');
            }
            return file_name == nullptr ? file_path : file_name + 1;
        };
        
        const auto to_string = [](const auto &value) -> std::string
        {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        };
        
        const char *tag = tags[static_cast<int>(level)];
        
        auto string_args = std::make_tuple(to_string(std::forward<Args>(args))...);
        const std::string formatted_message = std::vformat(message,
                                                           std::apply(
                                                           [](auto &&...s_args)
                                                           {
                                                               return std::make_format_args(s_args...);
                                                           },
                                                           std::move(string_args)));
        
        constexpr auto color_reset = "\033[0m";
        auto color = [level, color_reset]()
        {
            switch (level)
            {
            case LogLevel::Fatal:
            case LogLevel::Error:
                return "\033[31m"; // red
            case LogLevel::Warn:
                return "\033[33m"; // yellow
            default:
                return color_reset;
            }
        }();
        
        std::println("{}[{}] {}({}): {}{}", color, tag, get_file_name(file_path), line, formatted_message, color_reset);
    }
}

#define MH_FATAL(message, ...) mellohi::log(mellohi::LogLevel::Fatal, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define MH_ERROR(message, ...) mellohi::log(mellohi::LogLevel::Error, __FILE__, __LINE__, message, ##__VA_ARGS__)

#ifdef MH_LOG_WARN_ENABLED
    #define MH_WARN(message, ...) mellohi::log(mellohi::LogLevel::Warn, __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
    #define MH_WARN(message, ...)
#endif

#ifdef MH_LOG_INFO_ENABLED
    #define MH_INFO(message, ...) mellohi::log(mellohi::LogLevel::Info, __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
    #define MH_INFO(message, ...)
#endif

#ifdef MH_LOG_DEBUG_ENABLED
    #define MH_DEBUG(message, ...) mellohi::log(mellohi::LogLevel::Debug, __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
    #define MH_DEBUG(message, ...)
#endif

#ifdef MH_LOG_TRACE_ENABLED
    #define MH_TRACE(message, ...) mellohi::log(mellohi::LogLevel::Trace, __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
    #define MH_TRACE(message, ...)
#endif

#define MH_ASSERT(condition, message, ...) \
    if (!(condition)) {                    \
        MH_FATAL(message, ##__VA_ARGS__);  \
        std::abort();                      \
    }

#ifdef MH_DEBUG_MODE
#define MH_ASSERT_DEBUG(condition, message, ...) \
    if (!(condition)) {                          \
        MH_FATAL(message, ##__VA_ARGS__);        \
        std::abort();                            \
    }
#else
    #define MH_ASSERT_DEBUG(condition, message, ...)
#endif
