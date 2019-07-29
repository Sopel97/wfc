#pragma once

#include <cstring>
#include <cstdint>

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

    [[nodiscard]] double approximateLog(double a)
    {
        static_assert(sizeof(double) == sizeof(std::int64_t));

        std::int64_t x;
        std::memcpy(&x, &a, sizeof(double));

        return (x - 4606921278410026770ll) * 1.539095918623324e-16;
    }

    [[nodiscard]] float approximateLog(float a)
    {
        return static_cast<float>(approximateLog(static_cast<double>(a)));
    }
}
