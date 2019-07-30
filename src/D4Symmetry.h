#pragma once

#include <array>
#include <cstdint>
#include <type_traits>

#include "Enum.h"
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

template <>
struct EnumTraits<D4Symmetry>
{
    using IdType = int;

    static constexpr int cardinality = 7;

    // doesn't include identity (Rotation0)
    static constexpr std::array<D4Symmetry, 7> values{
            D4Symmetry::Rotation90,
            D4Symmetry::Rotation180,
            D4Symmetry::Rotation270,
            D4Symmetry::FlipAboutHorizontalAxis,
            D4Symmetry::FlipAboutVerticalAxis,
            D4Symmetry::FlipAboutMainDiagonal,
            D4Symmetry::FlipAboutAntiDiagonal
    };

    [[nodiscard]] static constexpr int toId(D4Symmetry s) noexcept
    {
        return static_cast<IdType>(s);
    }

    [[nodiscard]] static constexpr D4Symmetry fromId(IdType id) noexcept
    {
        return static_cast<D4Symmetry>(id);
    }
};

namespace detail
{
    [[nodiscard]] static constexpr auto asFlag(D4Symmetry s)
    {
        using T = std::underlying_type_t<D4Symmetry>;
        if (s == D4Symmetry::Rotation0) return static_cast<T>(0);
        return static_cast<T>(static_cast<T>(1) << (static_cast<T>(s) - 1));
    }
}

enum struct D4Symmetries : std::uint8_t
{
    None = 0,

    Rotation90 = detail::asFlag(D4Symmetry::Rotation90),
    Rotation180 = detail::asFlag(D4Symmetry::Rotation180),
    Rotation270 = detail::asFlag(D4Symmetry::Rotation270),
    FlipAboutHorizontalAxis = detail::asFlag(D4Symmetry::FlipAboutHorizontalAxis),
    FlipAboutVerticalAxis = detail::asFlag(D4Symmetry::FlipAboutVerticalAxis),
    FlipAboutMainDiagonal = detail::asFlag(D4Symmetry::FlipAboutMainDiagonal),
    FlipAboutAntiDiagonal = detail::asFlag(D4Symmetry::FlipAboutAntiDiagonal),

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

template <>
struct EnumTraits<D4Symmetries>
{
    using IdType = int;

    [[nodiscard]] static constexpr int toId(D4Symmetries s) noexcept
    {
        return static_cast<IdType>(s);
    }

    [[nodiscard]] static constexpr D4Symmetries fromId(IdType id) noexcept
    {
        return static_cast<D4Symmetries>(id);
    }
};

enum struct D4SymmetriesClosure : std::uint8_t
{
    /* P */ None = 0,

    /* N */ R180 = toId(D4Symmetries::Rotation180),
    /* C */ H = toId(D4Symmetries::FlipAboutHorizontalAxis),
    /* T */ V = toId(D4Symmetries::FlipAboutVerticalAxis),
    /* Q */ D = toId(D4Symmetries::FlipAboutMainDiagonal),
    /* L */ A = toId(D4Symmetries::FlipAboutAntiDiagonal),
    /* I */ R180_H_V = R180 | H | V,
    /* % */ R180_D_A = R180 | D | A,

    /*   */ AllRotations = toId(D4Symmetries::AllRotations),

    /* X */ All = toId(D4Symmetries::All)
};

template <>
struct EnumTraits<D4SymmetriesClosure>
{
    using IdType = int;

    [[nodiscard]] static constexpr int toId(D4SymmetriesClosure s) noexcept
    {
        return static_cast<IdType>(s);
    }

    [[nodiscard]] static constexpr D4SymmetriesClosure fromId(IdType id) noexcept
    {
        return static_cast<D4SymmetriesClosure>(id);
    }
};

[[nodiscard]] constexpr D4Symmetries operator|(D4Symmetries lhs, D4Symmetries rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return static_cast<D4Symmetries>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

[[nodiscard]] constexpr D4Symmetries operator|(D4Symmetries lhs, D4Symmetry rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return static_cast<D4Symmetries>(static_cast<T>(lhs) | detail::asFlag(rhs));
}

constexpr D4Symmetries& operator|=(D4Symmetries& lhs, D4Symmetries rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}

constexpr D4Symmetries& operator|=(D4Symmetries& lhs, D4Symmetry rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}

[[nodiscard]] constexpr D4Symmetries operator&(D4Symmetries lhs, D4Symmetries rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return static_cast<D4Symmetries>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

[[nodiscard]] constexpr D4Symmetries operator&(D4Symmetries lhs, D4Symmetry rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return static_cast<D4Symmetries>(static_cast<T>(lhs) & detail::asFlag(rhs));
}

constexpr D4Symmetries& operator&=(D4Symmetries& lhs, D4Symmetries rhs) noexcept
{
    lhs = lhs & rhs;
    return lhs;
}

constexpr D4Symmetries& operator&=(D4Symmetries& lhs, D4Symmetry rhs) noexcept
{
    lhs = lhs & rhs;
    return lhs;
}

[[nodiscard]] constexpr D4Symmetries operator^(D4Symmetries lhs, D4Symmetries rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return static_cast<D4Symmetries>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

[[nodiscard]] constexpr D4Symmetries operator^(D4Symmetries lhs, D4Symmetry rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return static_cast<D4Symmetries>(static_cast<T>(lhs) ^ detail::asFlag(rhs));
}

constexpr D4Symmetries& operator^=(D4Symmetries& lhs, D4Symmetries rhs) noexcept
{
    lhs = lhs ^ rhs;
    return lhs;
}

constexpr D4Symmetries& operator^=(D4Symmetries& lhs, D4Symmetry rhs) noexcept
{
    lhs = lhs ^ rhs;
    return lhs;
}

[[nodiscard]] constexpr bool contains(D4Symmetries lhs, D4Symmetries rhs) noexcept
{
    return (lhs & rhs) == rhs;
}

[[nodiscard]] constexpr bool contains(D4Symmetries lhs, D4Symmetry rhs) noexcept
{
    return (lhs & rhs) != D4Symmetries::None;
}

// returns the resulting symmetry of applying s1 after s2
// ie. s1(s2(x))
[[nodiscard]] constexpr D4Symmetry compose(D4Symmetry s1, D4Symmetry s2) noexcept
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

    return compositions[toId(s1)][toId(s2)];
}

// computes closure but only considers pairs (any from ss1, any from ss2) and vice versa
[[nodiscard]] constexpr D4Symmetries biclosure(D4Symmetries ss1, D4Symmetries ss2) noexcept
{
    D4Symmetries ss = ss1 | ss2;

    for (;;)
    {
        D4Symmetries next = ss;

        for (D4Symmetry s1 : values<D4Symmetry>())
        {
            if (!contains(ss1, s1))
            {
                continue;
            }

            for (D4Symmetry s2 : values<D4Symmetry>())
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

// computes closure but only considers pairs (any from ss1, s2) and vice versa
[[nodiscard]] constexpr D4Symmetries biclosure(D4Symmetries ss1, D4Symmetry s2) noexcept
{
    D4Symmetries ss = ss1 | s2;

    for (;;)
    {
        D4Symmetries next = ss;

        for (D4Symmetry s1 : values<D4Symmetry>())
        {
            if (!contains(ss1, s1))
            {
                continue;
            }

            next |= compose(s1, s2);
            next |= compose(s2, s1);
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
[[nodiscard]] constexpr D4SymmetriesClosure closure(D4Symmetries ss) noexcept
{
    return static_cast<D4SymmetriesClosure>(biclosure(ss, ss));
}

// returns true if ss == closure(ss)
[[nodiscard]] constexpr bool isClosed(D4Symmetries ss)
{
    D4Symmetries next = ss;

    for (D4Symmetry s1 : values<D4Symmetry>())
    {
        if (!contains(ss, s1))
        {
            continue;
        }

        for (D4Symmetry s2 : values<D4Symmetry>())
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

// when we transform a square using symmetry s then side that was in direction dir
// becomes side in direction ret[dir]
[[nodiscard]] constexpr ByDirection<Direction> mapping(D4Symmetry s) noexcept
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

    return mappings[toId(s)];
}

// inverse of mapping (as if done by inverse symmetry)
// only rotation90 and rotation270 need to be adjusted, the rest is reversible
[[nodiscard]] constexpr ByDirection<Direction> invMapping(D4Symmetry s) noexcept
{
    constexpr Direction N = Direction::North;
    constexpr Direction E = Direction::East;
    constexpr Direction S = Direction::South;
    constexpr Direction W = Direction::West;

    constexpr std::array<ByDirection<Direction>, 8> mappings{
        ByDirection<Direction>::nesw(N, E, S, W),
        ByDirection<Direction>::nesw(E, S, W, N),
        ByDirection<Direction>::nesw(S, W, N, E),
        ByDirection<Direction>::nesw(W, N, E, S),
        ByDirection<Direction>::nesw(S, E, N, W),
        ByDirection<Direction>::nesw(N, W, S, E),
        ByDirection<Direction>::nesw(W, S, E, N),
        ByDirection<Direction>::nesw(E, N, W, S)
    };

    return mappings[toId(s)];
}

[[nodiscard]] constexpr D4Symmetry inverse(D4Symmetry s) noexcept
{
    constexpr std::array<D4Symmetry, 8> inv{
        D4Symmetry::Rotation0,
        D4Symmetry::Rotation270,
        D4Symmetry::Rotation180,
        D4Symmetry::Rotation90,
        D4Symmetry::FlipAboutHorizontalAxis,
        D4Symmetry::FlipAboutVerticalAxis,
        D4Symmetry::FlipAboutMainDiagonal,
        D4Symmetry::FlipAboutAntiDiagonal
    };

    return inv[toId(s)];
}

// returns all and only symmetries (`m`) that produce distinct transforms that are
// not obtainable from any composition of symmetries from `ss`
// ie. given something with symmetries `ss` what other symmetries are needed to
// generate all missing distinct 'images'
// see static_asserts below for examples
[[nodiscard]] constexpr D4Symmetries missing(D4SymmetriesClosure ss) noexcept
{
    const D4Symmetries ssc = static_cast<D4Symmetries>(ss);
    D4Symmetries bic = ssc;

    D4Symmetries m = D4Symmetries::None;

    for (D4Symmetry s : values<D4Symmetry>())
    {
        if (!contains(bic, s))
        {
            m |= s;
            bic |= biclosure(ssc, s);
        }
    }

    return m;
}

[[nodiscard]] constexpr D4SymmetriesClosure operator|(D4SymmetriesClosure lhs, D4Symmetry rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return closure(static_cast<D4Symmetries>(static_cast<T>(lhs) | detail::asFlag(rhs)));
}

constexpr D4SymmetriesClosure& operator|=(D4SymmetriesClosure& lhs, D4Symmetry rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}

[[nodiscard]] constexpr D4SymmetriesClosure operator&(D4SymmetriesClosure lhs, D4Symmetry rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return closure(static_cast<D4Symmetries>(static_cast<T>(lhs) & detail::asFlag(rhs)));
}

constexpr D4SymmetriesClosure& operator&=(D4SymmetriesClosure& lhs, D4Symmetry rhs) noexcept
{
    lhs = lhs & rhs;
    return lhs;
}

[[nodiscard]] constexpr D4SymmetriesClosure operator^(D4SymmetriesClosure lhs, D4Symmetry rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return closure(static_cast<D4Symmetries>(static_cast<T>(lhs) ^ detail::asFlag(rhs)));
}

constexpr D4SymmetriesClosure& operator^=(D4SymmetriesClosure& lhs, D4Symmetry rhs) noexcept
{
    lhs = lhs ^ rhs;
    return lhs;
}

[[nodiscard]] constexpr D4SymmetriesClosure operator|(D4SymmetriesClosure lhs, D4Symmetries rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return closure(static_cast<D4Symmetries>(static_cast<T>(lhs) | static_cast<T>(rhs)));
}

constexpr D4SymmetriesClosure& operator|=(D4SymmetriesClosure& lhs, D4Symmetries rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}

[[nodiscard]] constexpr D4SymmetriesClosure operator&(D4SymmetriesClosure lhs, D4Symmetries rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return closure(static_cast<D4Symmetries>(static_cast<T>(lhs) & static_cast<T>(rhs)));
}

constexpr D4SymmetriesClosure& operator&=(D4SymmetriesClosure& lhs, D4Symmetries rhs) noexcept
{
    lhs = lhs & rhs;
    return lhs;
}

[[nodiscard]] constexpr D4SymmetriesClosure operator^(D4SymmetriesClosure lhs, D4Symmetries rhs) noexcept
{
    using T = std::underlying_type_t<D4Symmetries>;
    return closure(static_cast<D4Symmetries>(static_cast<T>(lhs) ^ static_cast<T>(rhs)));
}

constexpr D4SymmetriesClosure& operator^=(D4SymmetriesClosure& lhs, D4Symmetries rhs) noexcept
{
    lhs = lhs ^ rhs;
    return lhs;
}

// returns true iff for every `x` with symmetries `ss`: s1(x) == s2(x)
[[nodiscard]] static constexpr bool areEquivalentUnderSymmetries(D4SymmetriesClosure ss, D4Symmetry s1, D4Symmetry s2)
{
    return (ss | s1) == (ss | s2);
}

[[nodiscard]] static constexpr bool isRotation(D4Symmetry s)
{
    return toId(s) <= toId(D4Symmetry::Rotation270);
}

[[nodiscard]] static constexpr bool isMirroring(D4Symmetry s)
{
    return !isRotation(s);
}

struct D4SymmetryHelper
{
    [[nodiscard]] static constexpr D4SymmetriesClosure closureFromChar(char c) noexcept
    {
        switch (c)
        {
        case 'P':
            return D4SymmetriesClosure::None;
        case 'N':
            return D4SymmetriesClosure::R180;
        case 'C':
            return D4SymmetriesClosure::H;
        case 'T':
            return D4SymmetriesClosure::V;
        case 'Q':
            return D4SymmetriesClosure::D;
        case 'L':
            return D4SymmetriesClosure::A;
        case 'I':
            return D4SymmetriesClosure::R180_H_V;
        case '%':
            return D4SymmetriesClosure::R180_D_A;
        case 'X':
            return D4SymmetriesClosure::All;
        default:
            return D4SymmetriesClosure::AllRotations; // don't know what to do with it as there is no appropriate character
        }
    }
};

// The following have different possible results. Here we only test for the current implementation.
static_assert(missing(D4SymmetryHelper::closureFromChar('I')) == D4Symmetries::Rotation90);
static_assert(missing(D4SymmetryHelper::closureFromChar('%')) == D4Symmetries::Rotation90);
static_assert(missing(D4SymmetryHelper::closureFromChar('L')) == D4Symmetries::AllRotations);
static_assert(missing(D4SymmetryHelper::closureFromChar('T')) == D4Symmetries::AllRotations);
static_assert(missing(D4SymmetryHelper::closureFromChar('X')) == D4Symmetries::None);
static_assert(missing(D4SymmetryHelper::closureFromChar('P')) == D4Symmetries::All);
static_assert(missing(D4SymmetriesClosure::None) == D4Symmetries::All);

static_assert((D4SymmetryHelper::closureFromChar('I') | D4Symmetry::Rotation90) == D4SymmetriesClosure::All);

static_assert(areEquivalentUnderSymmetries(D4SymmetryHelper::closureFromChar('I'), D4Symmetry::Rotation90, D4Symmetry::FlipAboutAntiDiagonal));
static_assert(areEquivalentUnderSymmetries(D4SymmetryHelper::closureFromChar('I'), D4Symmetry::Rotation90, D4Symmetry::FlipAboutMainDiagonal));

template <typename Func>
void forEach(D4Symmetries ss, Func&& func)
{
    for (D4Symmetry s : values<D4Symmetry>())
    {
        if (contains(ss, s))
        {
            func(s);
        }
    }
}
