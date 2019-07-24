#pragma once

#include <array>
#include <cstdint>
#include <type_traits>

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
