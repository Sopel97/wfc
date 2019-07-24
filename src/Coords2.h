#pragma once

template <typename T>
constexpr int abs(T v)
{
    return v < 0 ? -v : v;
}

template <typename T>
constexpr int sign(T v)
{
    return (v > 0) - (v < 0);
}

template <typename T>
struct Coords2
{
    T x, y;

    constexpr Coords2() noexcept :
        x(0),
        y(0)
    {
    }

    constexpr Coords2(T x, T y) noexcept :
        x(x),
        y(y)
    {
    }

    constexpr Coords2& operator+=(const Coords2& rhs) noexcept
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    constexpr Coords2& operator-=(const Coords2& rhs) noexcept
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    constexpr Coords2& operator*=(T n) noexcept
    {
        x *= n;
        y *= n;
        return *this;
    }

    [[nodiscard]] constexpr friend Coords2 operator+(const Coords2& lhs, const Coords2& rhs) noexcept
    {
        return Coords2(lhs.x + rhs.x, lhs.y + rhs.y);
    }

    [[nodiscard]] constexpr friend Coords2 operator-(const Coords2& lhs, const Coords2& rhs) noexcept
    {
        return Coords2(lhs.x - rhs.x, lhs.y - rhs.y);
    }

    [[nodiscard]] constexpr friend Coords2 operator*(const Coords2& lhs, T n) noexcept
    {
        return Coords2(lhs.x * n, lhs.y * n);
    }

    [[nodiscard]] constexpr friend Coords2 operator*(T n, const Coords2& lhs) noexcept
    {
        return Coords2(lhs.x * n, lhs.y * n);
    }

    [[nodiscard]] constexpr friend Coords2 operator/(const Coords2& lhs, T n) noexcept
    {
        return Coords2(lhs.x / n, lhs.y / n);
    }

    [[nodiscard]] constexpr friend Coords2 operator-(const Coords2& lhs) noexcept
    {
        return Coords2(-lhs.x, -lhs.y);
    }

    [[nodiscard]] constexpr friend bool operator==(const Coords2& lhs, const Coords2& rhs) noexcept
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    [[nodiscard]] constexpr friend bool operator!=(const Coords2& lhs, const Coords2& rhs) noexcept
    {
        return !operator==(lhs, rhs);
    }

    [[nodiscard]] constexpr Coords2 sign() const noexcept
    {
        return Coords2(::sign(x), ::sign(y));
    }
};

using Coords2i = Coords2<int>;
using Coords2f = Coords2<float>;
