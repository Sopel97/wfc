#pragma once

#include <cstdint>

enum struct SquareSymmetries
{
    None = 0x00,
    Rotation90 = 0x01,
    Rotation180 = 0x02,
    Rotation270 = 0x04,
    FlipAboutHorizontalAxis = 0x08,
    FlipAboutVerticalAxis = 0x10,
    FlipAboutMainDiagonal = 0x20,
    FlipAboutAntiDiagonal = 0x40,

    AllRotations =
        Rotation90
        | Rotation180
        | Rotation270,

    AllFlips =
        FlipAboutHorizontalAxis
        | FlipAboutVerticalAxis
        | FlipAboutMainDiagonal
        | FlipAboutAntiDiagonal,

    All =
        AllRotations
        | AllFlips,
};

[[nodiscard]] static constexpr SquareSymmetries operator|(SquareSymmetries lhs, SquareSymmetries rhs) noexcept
{
    using T = std::underlying_type_t<SquareSymmetries>;
    return static_cast<SquareSymmetries>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

[[nodiscard]] static constexpr SquareSymmetries operator&(SquareSymmetries lhs, SquareSymmetries rhs) noexcept
{
    using T = std::underlying_type_t<SquareSymmetries>;
    return static_cast<SquareSymmetries>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

[[nodiscard]] static constexpr bool contains(SquareSymmetries lhs, SquareSymmetries rhs) noexcept
{
    return (lhs & rhs) == rhs;
}
