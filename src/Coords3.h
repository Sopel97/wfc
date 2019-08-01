#pragma once

#include "Coords2.h"

template <typename T>
struct Coords3
{
    T x, y, z;

    constexpr Coords3() noexcept :
        x(0),
        y(0),
        z(0)
    {
    }

    constexpr Coords3(Coords2<T> c, T z) noexcept :
        x(c.x),
        y(c.y),
        z(z)
    {
    }

    constexpr Coords3(T x, T y, T z) noexcept :
        x(x),
        y(y),
        z(z)
    {
    }

    constexpr Coords3& operator+=(const Coords3& rhs) noexcept
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    constexpr Coords3& operator-=(const Coords3& rhs) noexcept
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    constexpr Coords3& operator*=(T n) noexcept
    {
        x *= n;
        y *= n;
        z *= n;
        return *this;
    }

    [[nodiscard]] constexpr friend Coords3 operator+(const Coords3& lhs, const Coords3& rhs) noexcept
    {
        return Coords2(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
    }

    [[nodiscard]] constexpr friend Coords3 operator-(const Coords3& lhs, const Coords3& rhs) noexcept
    {
        return Coords2(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
    }

    [[nodiscard]] constexpr friend Coords3 operator*(const Coords3& lhs, T n) noexcept
    {
        return Coords2(lhs.x * n, lhs.y * n, lhs.z * n);
    }

    [[nodiscard]] constexpr friend Coords3 operator*(T n, const Coords3& lhs) noexcept
    {
        return Coords2(lhs.x * n, lhs.y * n, lhs.z * n);
    }

    [[nodiscard]] constexpr friend Coords3 operator/(const Coords3& lhs, T n) noexcept
    {
        return Coords2(lhs.x / n, lhs.y / n, lhs.z / n);
    }

    [[nodiscard]] constexpr friend Coords3 operator-(const Coords3& lhs) noexcept
    {
        return Coords2(-lhs.x, -lhs.y, -lhs.z);
    }

    [[nodiscard]] constexpr friend bool operator==(const Coords3& lhs, const Coords3& rhs) noexcept
    {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
    }

    [[nodiscard]] constexpr friend bool operator!=(const Coords3& lhs, const Coords3& rhs) noexcept
    {
        return !operator==(lhs, rhs);
    }

    [[nodiscard]] constexpr Coords3 sign() const noexcept
    {
        return Coords2(util::sign(x), util::sign(y), util::sign(z));
    }
};

using Coords3i = Coords3<int>;
using Coords3f = Coords3<float>;
