#pragma once

#include <array>
#include <chrono>
#include <iostream>
#include <string>
#include <string_view>

// member functions should not be called directly
// use LOG_* macros
struct Logger
{
    enum Level
    {
        None,

        Debug,
        Info,
        Warning,
        Error,
        Fatal,
        Always
    };

    static constexpr bool enabled = true;
    static constexpr Level minLevel = Info;

    template <typename... ArgTs>
    void logWithFooter(Level level, ArgTs&& ... args)
    {
        (*m_stream) << footer(level);
        log(level, std::forward<ArgTs>(args)...);
    }

    template <typename ArgT, typename... ArgTs>
    void log(Level level, ArgT&& arg, ArgTs&& ... args)
    {
        (*m_stream) << arg;
        log(level, std::forward<ArgTs>(args)...);
    }

    template <typename ArgT>
    void log(Level level, ArgT&& arg)
    {
        (*m_stream) << arg;
    }

    void setOutput(std::ostream& s)
    {
        m_stream = &s;
    }

    [[nodiscard]] static constexpr bool shouldLog(Level level)
    {
        return enabled && level >= minLevel;
    }

    [[nodiscard]] static std::string footer(Level level)
    {
        return "[" + std::to_string(time()) + " " + levelToString(level) + "] ";
    }

    [[nodiscard]] static std::uint64_t time()
    {
        return std::chrono::steady_clock::now().time_since_epoch().count();
    }

    [[nodiscard]] static constexpr const char* levelToString(Level level)
    {
        constexpr std::array<const char*, 7> v{
            "NONE",
            "DEBUG",
            "INFO",
            "WARNING",
            "ERROR",
            "FATAL",
            "ALWAYS"
        };

        return v[level];
    }

private:
    std::ostream* m_stream = &std::cout;
};

inline Logger g_logger;

#define LOG(logger, level, ...) if constexpr (Logger::shouldLog((level))) logger .logWithFooter(level, __VA_ARGS__, '\n');

#define LOG_DEBUG(logger, ...) LOG(logger, Logger::Debug, __VA_ARGS__);
#define LOG_INFO(logger, ...) LOG(logger, Logger::Info, __VA_ARGS__);
#define LOG_WARNING(logger, ...) LOG(logger, Logger::Warning, __VA_ARGS__);
#define LOG_ERROR(logger, ...) LOG(logger, Logger::Error, __VA_ARGS__);
#define LOG_FATAL(logger, ...) LOG(logger, Logger::Fatal, __VA_ARGS__);
#define LOG_ALWAYS(logger, ...) LOG(logger, Logger::Always, __VA_ARGS__);
