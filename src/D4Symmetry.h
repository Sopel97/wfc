#pragma once

#include <array>
#include <cstdint>
#include <type_traits>

#include "Direction.h"

enum struct D4Symmetry : std::uint8_t
{
    Rotation90 = 0,
    Rotation180,
    Rotation270,
    FlipAboutHorizontalAxis,
    FlipAboutVerticalAxis,
    FlipAboutMainDiagonal,
    FlipAboutAntiDiagonal
};

struct D4SymmetryHelper
{
    [[nodiscard]] static constexpr int toId(D4Symmetry s)
    {
        return static_cast<int>(s);
    }

    [[nodiscard]] static constexpr D4Symmetry fromId(int id)
    {
        return static_cast<D4Symmetry>(id);
    }

    [[nodiscard]] static const std::array<D4Symmetry, 7>& values()
    {
        static const std::array<D4Symmetry, 7> v{
            D4Symmetry::Rotation90,
            D4Symmetry::Rotation180,
            D4Symmetry::Rotation270,
            D4Symmetry::FlipAboutHorizontalAxis,
            D4Symmetry::FlipAboutVerticalAxis,
            D4Symmetry::FlipAboutMainDiagonal,
            D4Symmetry::FlipAboutAntiDiagonal
        };

        return v;
    }

    // when we transform a square using symmetry s then side that was in direction dir
    // becomes side in direction ret[dir]
    [[nodiscard]] static const ByDirection<Direction>& mapping(D4Symmetry s)
    {
        static const std::array<ByDirection<Direction>, 7> mappings{
            ByDirection<Direction>::nesw(Direction::West, Direction::North, Direction::East, Direction::South),
            ByDirection<Direction>::nesw(Direction::South, Direction::West, Direction::North, Direction::East),
            ByDirection<Direction>::nesw(Direction::East, Direction::South, Direction::West, Direction::North),
            ByDirection<Direction>::nesw(Direction::South, Direction::East, Direction::North, Direction::West),
            ByDirection<Direction>::nesw(Direction::North, Direction::West, Direction::South, Direction::East),
            ByDirection<Direction>::nesw(Direction::West, Direction::South, Direction::East, Direction::North),
            ByDirection<Direction>::nesw(Direction::East, Direction::North, Direction::West, Direction::South)
        };

        return mappings[D4SymmetryHelper::toId(s)];
    }
};

[[nodiscard]] static constexpr auto asFlag(D4Symmetry s)
{
    using T = std::underlying_type_t<D4Symmetry>;
    return static_cast<T>(1) << static_cast<T>(s);
}

enum struct D4Symmetries : std::uint8_t
{
    None = 0,

    AllRotations =
        asFlag(D4Symmetry::Rotation90)
        | asFlag(D4Symmetry::Rotation180)
        | asFlag(D4Symmetry::Rotation270),

    AllFlips =
        asFlag(D4Symmetry::FlipAboutHorizontalAxis)
        | asFlag(D4Symmetry::FlipAboutVerticalAxis)
        | asFlag(D4Symmetry::FlipAboutMainDiagonal)
        | asFlag(D4Symmetry::FlipAboutAntiDiagonal),

    All =
        AllRotations
        | AllFlips,
};

[[nodiscard]] static constexpr D4Symmetries operator|(D4Symmetries lhs, D4Symmetries rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return static_cast<D4Symmetries>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

[[nodiscard]] static constexpr D4Symmetries operator|(D4Symmetries lhs, D4Symmetry rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return static_cast<D4Symmetries>(static_cast<T>(lhs) | asFlag(rhs));
}

[[nodiscard]] static constexpr D4Symmetries operator&(D4Symmetries lhs, D4Symmetries rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return static_cast<D4Symmetries>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

[[nodiscard]] static constexpr D4Symmetries operator&(D4Symmetries lhs, D4Symmetry rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return static_cast<D4Symmetries>(static_cast<T>(lhs) & asFlag(rhs));
}

[[nodiscard]] static constexpr bool contains(D4Symmetries lhs, D4Symmetries rhs) noexcept
{
    return (lhs & rhs) == rhs;
}

[[nodiscard]] static constexpr bool contains(D4Symmetries lhs, D4Symmetry rhs) noexcept
{
    return (lhs & rhs) != D4Symmetries::None;
}

template <typename Func>
void forEach(D4Symmetries ss, Func&& func)
{
    for (D4Symmetry s : D4SymmetryHelper::values())
    {
        if (contains(ss, s))
        {
            func(s);
        }
    }
}
