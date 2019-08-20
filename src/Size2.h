#pragma once

template <typename T>
struct Size2
{
    T width, height;

    constexpr Size2() noexcept :
        width(0),
        height(0)
    {
    }

    constexpr Size2(T width, T height) noexcept :
        width(width),
        height(height)
    {
    }

    constexpr Size2& operator*=(T n) noexcept
    {
        width *= n;
        height *= n;
        return *this;
    }

    [[nodiscard]] constexpr T total() const noexcept
    {
        return width * height;
    }

    [[nodiscard]] constexpr friend Size2 operator+(const Size2& lhs, const Size2& rhs) noexcept
    {
        return Size2(lhs.width + rhs.width, lhs.height + rhs.height);
    }

    [[nodiscard]] constexpr friend Size2 operator*(const Size2& lhs, const Size2& rhs) noexcept
    {
        return Size2(lhs.width * rhs.width, lhs.height * rhs.height);
    }

    [[nodiscard]] constexpr friend Size2 operator*(const Size2& lhs, T n) noexcept
    {
        return Size2(lhs.width * n, lhs.height * n);
    }

    [[nodiscard]] constexpr friend Size2 operator*(T n, const Size2& lhs) noexcept
    {
        return Size2(lhs.width * n, lhs.height * n);
    }

    [[nodiscard]] constexpr friend Size2 operator/(const Size2& lhs, T n) noexcept
    {
        return Size2(lhs.width / n, lhs.height / n);
    }

    [[nodiscard]] constexpr friend bool operator==(const Size2& lhs, const Size2& rhs) noexcept
    {
        return lhs.width == rhs.width && lhs.height == rhs.height;
    }

    [[nodiscard]] constexpr friend bool operator!=(const Size2& lhs, const Size2& rhs) noexcept
    {
        return !operator==(lhs, rhs);
    }
};

using Size2i = Size2<int>;
using Size2f = Size2<float>;
