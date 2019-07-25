#pragma once

#include <array>
#include <cstdint>
#include <type_traits>

#include "Direction.h"

// http://facstaff.cbu.edu/wschrein/media/M402%20Notes/M402C1.pdf

enum struct D4Symmetry : std::uint8_t
{
    Rotation0 = 0,
    Rotation90,
    Rotation180,
    Rotation270,
    FlipAboutHorizontalAxis,
    FlipAboutVerticalAxis,
    FlipAboutMainDiagonal,
    FlipAboutAntiDiagonal
};

[[nodiscard]] static constexpr auto asFlag(D4Symmetry s)
{
    using T = std::underlying_type_t<D4Symmetry>;
    if (s == D4Symmetry::Rotation0) return static_cast<T>(0);
    return static_cast<T>(static_cast<T>(1) << (static_cast<T>(s)-1));
}

enum struct D4Symmetries : std::uint8_t
{
    None = 0,

    Rotation90 = asFlag(D4Symmetry::Rotation90),
    Rotation180 = asFlag(D4Symmetry::Rotation180),
    Rotation270 = asFlag(D4Symmetry::Rotation270),
    FlipAboutHorizontalAxis = asFlag(D4Symmetry::FlipAboutHorizontalAxis),
    FlipAboutVerticalAxis = asFlag(D4Symmetry::FlipAboutVerticalAxis),
    FlipAboutMainDiagonal = asFlag(D4Symmetry::FlipAboutMainDiagonal),
    FlipAboutAntiDiagonal = asFlag(D4Symmetry::FlipAboutAntiDiagonal),

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

static constexpr D4Symmetries& operator|=(D4Symmetries& lhs, D4Symmetries rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}

static constexpr D4Symmetries& operator|=(D4Symmetries& lhs, D4Symmetry rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
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

static constexpr D4Symmetries& operator&=(D4Symmetries& lhs, D4Symmetries rhs) noexcept
{
    lhs = lhs & rhs;
    return lhs;
}

static constexpr D4Symmetries& operator&=(D4Symmetries& lhs, D4Symmetry rhs) noexcept
{
    lhs = lhs & rhs;
    return lhs;
}

[[nodiscard]] static constexpr D4Symmetries operator^(D4Symmetries lhs, D4Symmetries rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return static_cast<D4Symmetries>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

[[nodiscard]] static constexpr D4Symmetries operator^(D4Symmetries lhs, D4Symmetry rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return static_cast<D4Symmetries>(static_cast<T>(lhs) ^ asFlag(rhs));
}

static constexpr D4Symmetries& operator^=(D4Symmetries& lhs, D4Symmetries rhs) noexcept
{
    lhs = lhs ^ rhs;
    return lhs;
}

static constexpr D4Symmetries& operator^=(D4Symmetries& lhs, D4Symmetry rhs) noexcept
{
    lhs = lhs ^ rhs;
    return lhs;
}

[[nodiscard]] static constexpr bool contains(D4Symmetries lhs, D4Symmetries rhs) noexcept
{
    return (lhs & rhs) == rhs;
}

[[nodiscard]] static constexpr bool contains(D4Symmetries lhs, D4Symmetry rhs) noexcept
{
    return (lhs & rhs) != D4Symmetries::None;
}

struct D4SymmetryHelper
{
    [[nodiscard]] static constexpr D4Symmetries fromChar(char c)
    {
        switch (c)
        {
        case 'X':
            return D4Symmetries::All;
        case 'I':
            return D4Symmetries::Rotation180 | D4Symmetry::FlipAboutHorizontalAxis | D4Symmetry::FlipAboutVerticalAxis;
        case 'T':
            return D4Symmetries::FlipAboutVerticalAxis;
        case '/':
            return D4Symmetries::Rotation180 | D4Symmetry::FlipAboutMainDiagonal | D4Symmetry::FlipAboutAntiDiagonal;
        case 'L':
            return D4Symmetries::FlipAboutAntiDiagonal;
        default:
            return D4Symmetries::None;
        }
    }

    [[nodiscard]] static constexpr int toId(D4Symmetry s)
    {
        return static_cast<int>(s);
    }

    [[nodiscard]] static constexpr D4Symmetry fromId(int id)
    {
        return static_cast<D4Symmetry>(id);
    }

    // doesn't include identity (Rotation0)
    [[nodiscard]] static constexpr std::array<D4Symmetry, 7> values()
    {
        constexpr std::array<D4Symmetry, 7> v{
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
    [[nodiscard]] static constexpr ByDirection<Direction> mapping(D4Symmetry s)
    {
        constexpr Direction N = Direction::North;
        constexpr Direction E = Direction::East;
        constexpr Direction S = Direction::South;
        constexpr Direction W = Direction::West;

        constexpr std::array<ByDirection<Direction>, 8> mappings{
            ByDirection<Direction>::nesw(N, E, S, W),
            ByDirection<Direction>::nesw(W, N, E, S),
            ByDirection<Direction>::nesw(S, W, N, E),
            ByDirection<Direction>::nesw(E, S, W, N),
            ByDirection<Direction>::nesw(S, E, N, W),
            ByDirection<Direction>::nesw(N, W, S, E),
            ByDirection<Direction>::nesw(W, S, E, N),
            ByDirection<Direction>::nesw(E, N, W, S)
        };

        return mappings[D4SymmetryHelper::toId(s)];
    }

    // returns the resulting symmetry of applying s1 after s2
    // ie. s1(s2(x))
    [[nodiscard]] static constexpr D4Symmetry compose(D4Symmetry s1, D4Symmetry s2)
    {
        constexpr D4Symmetry R0 = D4Symmetry::Rotation0;
        constexpr D4Symmetry R90 = D4Symmetry::Rotation90;
        constexpr D4Symmetry R180 = D4Symmetry::Rotation180;
        constexpr D4Symmetry R270 = D4Symmetry::Rotation270;
        constexpr D4Symmetry H = D4Symmetry::FlipAboutHorizontalAxis;
        constexpr D4Symmetry V = D4Symmetry::FlipAboutVerticalAxis;
        constexpr D4Symmetry D = D4Symmetry::FlipAboutMainDiagonal;
        constexpr D4Symmetry A = D4Symmetry::FlipAboutAntiDiagonal;

        constexpr D4Symmetry compositions[8][8]{
            { R0,   R90,  R180, R270, H,    V,    D,    A    },
            { R90,  R180, R270, R0,   A,    D,    H,    V    },
            { R180, R270, R0,   R90,  V,    H,    A,    D    },
            { R270, R0,   R90,  R180, D,    A,    V,    H    },
            { H,    D,    V,    A,    R0,   R180, R90,  R270 },
            { V,    A,    H,    D,    R180, R0,   R270, R90  },
            { D,    V,    A,    H,    R270, R90,  R0,  R180  },
            { A,    H,    D,    V,    R90,  R270, R180, R0   }
        };

        return compositions[D4SymmetryHelper::toId(s1)][D4SymmetryHelper::toId(s2)];
    }

    // computes closure but only considers pairs of symmetries from 2 different sets
    [[nodiscard]] static constexpr D4Symmetries biclosure(D4Symmetries ss1, D4Symmetries ss2)
    {
        D4Symmetries ss = ss1 | ss2;

        for (;;)
        {
            D4Symmetries next = ss;

            for (D4Symmetry s1 : D4SymmetryHelper::values())
            {
                if (!contains(ss1, s1))
                {
                    continue;
                }

                for (D4Symmetry s2 : D4SymmetryHelper::values())
                {
                    if (contains(ss2, s2))
                    {
                        next |= compose(s1, s2);
                        next |= compose(s2, s1);
                    }
                }
            }

            if (next == ss)
            {
                return next;
            }

            ss = next;
        }
    }

    // returns all symmetries that are induced by `ss`
    // equivalent to biclosure(ss, ss)
    // TODO: maybe precompute?
    [[nodiscard]] static constexpr D4Symmetries closure(D4Symmetries ss)
    {
        return biclosure(ss, ss);
    }

    // returns true if ss == closure(ss)
    [[nodiscard]] static constexpr bool isClosed(D4Symmetries ss)
    {
        D4Symmetries next = ss;

        for (D4Symmetry s1 : D4SymmetryHelper::values())
        {
            if (!contains(ss, s1))
            {
                continue;
            }

            for (D4Symmetry s2 : D4SymmetryHelper::values())
            {
                if (contains(ss, s2))
                {
                    next |= compose(s1, s2);
                    next |= compose(s2, s1);
                }
            }
        }

        return next == ss;
    }

    // returns all and only symmetries (`m`) that produce distinct transforms that are
    // not obtainable from any composition of symmetries from `ss`
    // ie. given something with symmetries `ss` what other symmetries are needed to
    // generate all missing distinct 'images'
    // see static_asserts below for examples
    [[nodiscard]] static constexpr D4Symmetries missing(D4Symmetries ss)
    {
        const D4Symmetries ssc = closure(ss);
        D4Symmetries bic = ssc;

        D4Symmetries m = D4Symmetries::None;

        for (D4Symmetry s : D4SymmetryHelper::values())
        {
            if (!contains(bic, s))
            {
                m |= s;
                bic |= biclosure(ssc, m);
            }
        }

        return m;
    }
};

static_assert(D4SymmetryHelper::isClosed(D4SymmetryHelper::fromChar('I')));
static_assert(D4SymmetryHelper::isClosed(D4SymmetryHelper::fromChar('T')));
static_assert(D4SymmetryHelper::isClosed(D4SymmetryHelper::fromChar('X')));
static_assert(D4SymmetryHelper::isClosed(D4SymmetryHelper::fromChar('/')));
static_assert(D4SymmetryHelper::isClosed(D4SymmetryHelper::fromChar('L')));
static_assert(D4SymmetryHelper::isClosed(D4SymmetryHelper::fromChar('P')));

static_assert(D4SymmetryHelper::missing(D4SymmetryHelper::fromChar('I')) == (D4Symmetries::None | D4Symmetry::Rotation90));
static_assert(D4SymmetryHelper::missing(D4SymmetryHelper::fromChar('/')) == (D4Symmetries::None | D4Symmetry::Rotation90));
static_assert(D4SymmetryHelper::missing(D4SymmetryHelper::fromChar('L')) == D4Symmetries::AllRotations);
static_assert(D4SymmetryHelper::missing(D4SymmetryHelper::fromChar('T')) == D4Symmetries::AllRotations);
static_assert(D4SymmetryHelper::missing(D4SymmetryHelper::fromChar('X')) == D4Symmetries::None);
static_assert(D4SymmetryHelper::missing(D4SymmetryHelper::fromChar('P')) == D4Symmetries::All);

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
