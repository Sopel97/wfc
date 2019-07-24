#pragma once

#include "Size2.h"

template <typename T>
struct Size3
{
    T width, height, depth;

    constexpr Size3() noexcept :
        width(0),
        height(0),
        depth(0)
    {
    }

    constexpr Size3(Size2<T> wh, T depth) noexcept :
        width(wh.width),
        height(wh.height),
        depth(depth)
    {
    }

    constexpr Size3(T width, T height, T depth) noexcept :
        width(width),
        height(height),
        depth(depth)
    {
    }

    constexpr Size3& operator*=(T n) noexcept
    {
        width *= n;
        height *= n;
        depth *= n;
        return *this;
    }

    [[nodiscard]] constexpr T total() const noexcept
    {
        return width * height * depth;
    }

    [[nodiscard]] constexpr friend Size3 operator*(const Size3& lhs, T n) noexcept
    {
        return Size3(lhs.width * n, lhs.height * n, lhs.depth * n);
    }

    [[nodiscard]] constexpr friend Size3 operator*(T n, const Size3& lhs) noexcept
    {
        return Size3(lhs.width * n, lhs.height * n, lhs.depth * n);
    }

    [[nodiscard]] constexpr friend Size3 operator/(const Size3& lhs, T n) noexcept
    {
        return Size3(lhs.width / n, lhs.height / n, lhs.depth / n);
    }

    [[nodiscard]] constexpr friend bool operator==(const Size3& lhs, const Size3& rhs) noexcept
    {
        return lhs.width == rhs.width && lhs.height == rhs.height && lhs.depth == rhs.depth;
    }

    [[nodiscard]] constexpr friend bool operator!=(const Size3& lhs, const Size3& rhs) noexcept
    {
        return !operator==(lhs, rhs);
    }
};

using Size3i = Size3<int>;
using Size3f = Size3<float>;
