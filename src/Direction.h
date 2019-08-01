#pragma once

#include <array>
#include <cstdint>

#include "Enum.h"
#include "Coords2.h"
#include "Util.h"

enum struct Direction : std::uint8_t
{
    North = 0,
    East,
    South,
    West
};

template <>
struct EnumTraits<Direction>
{
    using IdType = int;

    static constexpr int cardinality = 4;

    // doesn't include identity (Rotation0)
    static constexpr std::array<Direction, 4> values{
        Direction::North,
        Direction::East,
        Direction::South,
        Direction::West
    };

    [[nodiscard]] static constexpr int toId(Direction s) noexcept
    {
        return static_cast<IdType>(s);
    }

    [[nodiscard]] static constexpr Direction fromId(IdType id) noexcept
    {
        return static_cast<Direction>(id);
    }
};

template <>
[[nodiscard]] constexpr Direction fromId(int id) noexcept
{
    return static_cast<Direction>(id);
}

[[nodiscard]] constexpr Direction rotatedClockwise(Direction dir) noexcept
{
    return fromId<Direction>((toId(dir) + 1) % 4);
}

[[nodiscard]] constexpr Direction rotatedCounterClockwise(Direction dir) noexcept
{
    return fromId<Direction>((toId(dir) + 3) % 4);
}

[[nodiscard]] constexpr Direction oppositeTo(Direction dir) noexcept
{
    return fromId<Direction>((toId(dir) + 2) % 4);
}

[[nodiscard]] constexpr bool areOpposite(Direction d1, Direction d2) noexcept
{
    const int diff = toId(d1) - toId(d2);
    return util::abs(diff) == 2;
}

[[nodiscard]] constexpr bool areParallel(Direction d1, Direction d2) noexcept
{
    const int diff = toId(d1) - toId(d2);
    return diff == 0 || util::abs(diff) == 2;
}

[[nodiscard]] constexpr bool arePerpendicular(Direction d1, Direction d2) noexcept
{
    const int diff = toId(d1) - toId(d2);
    return util::abs(diff) == 1 || util::abs(diff) == 3;
}

[[nodiscard]] constexpr Coords2i offset(Direction dir) noexcept
{
    constexpr std::array<Coords2i, 4> offsets{
        Coords2i(0, -1),
        Coords2i(1, 0),
        Coords2i(0, 1),
        Coords2i(-1, 0)
    };

    return offsets[toId(dir)];
}

[[nodiscard]] constexpr std::string_view toString(Direction dir) noexcept
{
    constexpr std::array<std::string_view, 4> names{
        "North",
        "East",
        "South",
        "West"
    };

    return names[toId(dir)];
}

template <typename T>
struct ByDirection : std::array<T, 4>
{
    using BaseType = std::array<T, 4>;

    template <typename... FwdT>
    [[nodiscard]] static constexpr ByDirection<T> nesw(FwdT&& ... args)
    {
        return ByDirection(std::forward<FwdT>(args)...);
    }

    constexpr ByDirection() = default;
    constexpr ByDirection(const ByDirection&) = default;
    constexpr ByDirection(ByDirection&&) = default;
    constexpr ByDirection& operator=(const ByDirection&) = default;
    constexpr ByDirection& operator=(ByDirection&&) = default;

    [[nodiscard]] constexpr T& operator[](Direction dir)
    {
        return BaseType::operator[](toId(dir));
    }

    [[nodiscard]] constexpr const T& operator[](Direction dir) const
    {
        return BaseType::operator[](toId(dir));
    }

private:
    template <typename Fwd0T, typename Fwd1T, typename Fwd2T, typename Fwd3T>
    constexpr ByDirection(Fwd0T&& n, Fwd1T&& e, Fwd2T&& s, Fwd3T&& w) :
        BaseType{ std::forward<Fwd0T>(n), std::forward<Fwd1T>(e), std::forward<Fwd2T>(s), std::forward<Fwd3T>(w) }
    {

    }
};
