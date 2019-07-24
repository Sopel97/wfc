#pragma once

namespace util
{
    template <typename T>
    [[nodiscard]] constexpr int abs(T v)
    {
        return v < 0 ? -v : v;
    }

    template <typename T>
    [[nodiscard]] constexpr int sign(T v)
    {
        return (v > 0) - (v < 0);
    }
}
