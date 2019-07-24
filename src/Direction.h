#pragma once

#include <array>
#include <cstdint>

#include "Coords2.h"

enum struct Direction : std::uint8_t
{
    North = 0,
    East,
    South,
    West
};

struct DirectionHelper
{
    static int toId(Direction dir)
    {
        return static_cast<int>(dir);
    }

    static Direction fromId(int id)
    {
        return static_cast<Direction>(id);
    }

    static Direction rotatedClockwise(Direction dir)
    {
        return fromId((toId(dir) + 1) % 4);
    }

    static Direction rotatedCounterClockwise(Direction dir)
    {
        return fromId((toId(dir) + 3) % 4);
    }

    static Direction oppositeTo(Direction dir)
    {
        return fromId((toId(dir) + 2) % 4);
    }

    static bool areOpposite(Direction d1, Direction d2)
    {
        const int diff = toId(d1) - toId(d2);
        return abs(diff) == 2;
    }

    static bool areParallel(Direction d1, Direction d2)
    {
        const int diff = toId(d1) - toId(d2);
        return diff == 0 || abs(diff) == 2;
    }

    static bool arePerpendicular(Direction d1, Direction d2)
    {
        const int diff = toId(d1) - toId(d2);
        return abs(diff) == 1 || abs(diff) == 3;
    }

    static Coords2i offset(Direction dir)
    {
        static const std::array<Coords2i, 4> offsets{
            Coords2i(0, -1),
            Coords2i(1, 0),
            Coords2i(0, 1),
            Coords2i(-1, 0)
        };

        return offsets[toId(dir)];
    }

    static const std::array<Direction, 4>& values()
    {
        static const std::array<Direction, 4> v{
            Direction::North,
            Direction::East,
            Direction::South,
            Direction::West,
        };

        return v;
    }

    static const std::string& toString(Direction dir)
    {
        static const std::array<std::string, 4> names{
            "North",
            "East",
            "South",
            "West"
        };

        return names[toId(dir)];
    }
};

template <typename T>
struct ByDirection : std::array<T, 4>
{
    using BaseType = std::array<T, 4>;

    ByDirection() noexcept = default;
    ByDirection(const ByDirection&) noexcept = default;
    ByDirection(ByDirection&&) noexcept = default;
    ByDirection& operator=(const ByDirection&) noexcept = default;
    ByDirection& operator=(ByDirection&&) noexcept = default;

    T& operator[](Direction dir)
    {
        return BaseType::operator[](DirectionHelper::toId(dir));
    }

    const T& operator[](Direction dir) const
    {
        return BaseType::operator[](DirectionHelper::toId(dir));
    }
};
