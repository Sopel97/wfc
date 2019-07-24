#pragma once

#include <cstdint>

enum struct WrappingMode : std::uint8_t
{
    None = 0x0,
    Horizontal = 0x1,
    Vertical = 0x2,
    All = Horizontal | Vertical
};

[[nodiscard]] static constexpr WrappingMode operator|(WrappingMode lhs, WrappingMode rhs) noexcept
{
    using T = std::underlying_type_t<WrappingMode>;
    return static_cast<WrappingMode>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

[[nodiscard]] static constexpr WrappingMode operator&(WrappingMode lhs, WrappingMode rhs) noexcept
{
    using T = std::underlying_type_t<WrappingMode>;
    return static_cast<WrappingMode>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

[[nodiscard]] static constexpr bool contains(WrappingMode lhs, WrappingMode rhs) noexcept
{
    return (lhs & rhs) == rhs;
}
