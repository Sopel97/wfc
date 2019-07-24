#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

struct ColorRGBf
{
    constexpr ColorRGBf() noexcept :
        r{},
        g{},
        b{}
    {
    }

    constexpr ColorRGBf(float r, float g, float b) noexcept :
        r(r),
        g(g),
        b(b)
    {

    }

    constexpr ColorRGBf(const ColorRGBf&) noexcept = default;
    constexpr ColorRGBf(ColorRGBf&&) noexcept = default;
    constexpr ColorRGBf& operator=(const ColorRGBf&) noexcept = default;
    constexpr ColorRGBf& operator=(ColorRGBf&&) noexcept = default;

    constexpr ColorRGBf& operator+=(const ColorRGBf& rhs)
    {
        r += rhs.r;
        g += rhs.g;
        b += rhs.b;
        return *this;
    }

    constexpr ColorRGBf& operator*=(const ColorRGBf& rhs)
    {
        r *= rhs.r;
        g *= rhs.g;
        b *= rhs.b;
        return *this;
    }

    constexpr ColorRGBf& operator*=(float rhs)
    {
        r *= rhs;
        g *= rhs;
        b *= rhs;
        return *this;
    }

    [[nodiscard]] constexpr float total() const
    {
        return r + g + b;
    }

    [[nodiscard]] constexpr float max() const
    {
        return std::max(std::max(r, g), b);
    }

    [[nodiscard]] constexpr friend bool operator<(const ColorRGBf& lhs, const ColorRGBf& rhs) noexcept
    {
        if (lhs.r != rhs.r) return lhs.r < rhs.r;
        if (lhs.g != rhs.g) return lhs.g < rhs.g;
        if (lhs.b != rhs.b) return lhs.b < rhs.b;
        return false;
    }

    float r, g, b;
};

[[nodiscard]] constexpr ColorRGBf operator*(const ColorRGBf& lhs, float rhs)
{
    return ColorRGBf(lhs.r * rhs, lhs.g * rhs, lhs.b * rhs);
}

[[nodiscard]] constexpr ColorRGBf operator/(const ColorRGBf& lhs, float rhs)
{
    return ColorRGBf(lhs.r / rhs, lhs.g / rhs, lhs.b / rhs);
}

[[nodiscard]] constexpr ColorRGBf operator*(float lhs, const ColorRGBf& rhs)
{
    return ColorRGBf(lhs * rhs.r, lhs * rhs.g, lhs * rhs.b);
}

[[nodiscard]] constexpr ColorRGBf operator+(const ColorRGBf& lhs, const ColorRGBf& rhs)
{
    return ColorRGBf(lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b);
}

[[nodiscard]] constexpr ColorRGBf operator-(const ColorRGBf& lhs, const ColorRGBf& rhs)
{
    return ColorRGBf(lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b);
}

[[nodiscard]] constexpr ColorRGBf operator*(const ColorRGBf& lhs, const ColorRGBf& rhs)
{
    return ColorRGBf(lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b);
}

[[nodiscard]] inline ColorRGBf operator^(const ColorRGBf& lhs, float gamma)
{
    return ColorRGBf(
        std::pow(lhs.r, gamma),
        std::pow(lhs.g, gamma),
        std::pow(lhs.b, gamma)
    );
}

[[nodiscard]] constexpr ColorRGBf operator-(const ColorRGBf& lhs)
{
    return ColorRGBf(
        -lhs.r,
        -lhs.g,
        -lhs.b
    );
}

[[nodiscard]] inline ColorRGBf exp(const ColorRGBf& lhs)
{
    return ColorRGBf(
        std::exp(lhs.r),
        std::exp(lhs.g),
        std::exp(lhs.b)
    );
}

struct ColorRGBi
{
    constexpr ColorRGBi() noexcept :
        r{},
        g{},
        b{}
    {
    }

    constexpr ColorRGBi(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept :
        r(r),
        g(g),
        b(b)
    {

    }

    explicit ColorRGBi(const ColorRGBf& other) noexcept :
        r(static_cast<std::uint8_t>(std::clamp(other.r, 0.0f, 1.0f) * 255.0f + 0.5f)),
        g(static_cast<std::uint8_t>(std::clamp(other.g, 0.0f, 1.0f) * 255.0f + 0.5f)),
        b(static_cast<std::uint8_t>(std::clamp(other.b, 0.0f, 1.0f) * 255.0f + 0.5f))
    {

    }

    constexpr ColorRGBi(const ColorRGBi&) noexcept = default;
    constexpr ColorRGBi(ColorRGBi&&) noexcept = default;
    constexpr ColorRGBi& operator=(const ColorRGBi&) noexcept = default;
    constexpr ColorRGBi& operator=(ColorRGBi&&) noexcept = default;

    [[nodiscard]] constexpr friend bool operator<(const ColorRGBi& lhs, const ColorRGBi& rhs) noexcept
    {
        if (lhs.r != rhs.r) return lhs.r < rhs.r;
        if (lhs.g != rhs.g) return lhs.g < rhs.g;
        if (lhs.b != rhs.b) return lhs.b < rhs.b;
        return false;
    }

    [[nodiscard]] constexpr friend bool operator==(const ColorRGBi& lhs, const ColorRGBi& rhs) noexcept
    {
        return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
    }

    [[nodiscard]] constexpr friend bool operator!=(const ColorRGBi& lhs, const ColorRGBi& rhs) noexcept
    {
        return !operator==(lhs, rhs);
    }

    std::uint8_t r, g, b;
};
