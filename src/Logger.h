#pragma once

#include <iostream>

struct Logger
{
    static constexpr bool enabled = false;

    template <typename ArgT, typename... ArgTs>
    void log(ArgT&& arg, ArgTs&& ... args)
    {
        if (!enabled)
        {
            return;
        }

        (*m_stream) << arg;
        log(std::forward<ArgTs>(args)...);
    }

    template <typename ArgT>
    void log(ArgT&& arg)
    {
        if (!enabled)
        {
            return;
        }

        (*m_stream) << arg;
    }

private:
    std::ostream* m_stream = &std::cout;
};

inline Logger g_logger;

#define LOG(logger, ...) if constexpr (logger .enabled) logger .log(__VA_ARGS__);

