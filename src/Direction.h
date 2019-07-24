#pragma once

#include <array>
#include <cstdint>

#include "Coords2.h"
#include "Util.h"

enum struct Direction : std::uint8_t
{
    North = 0,
    East,
    South,
    West
};

struct DirectionHelper
{
    [[nodiscard]] static constexpr int toId(Direction dir)
    {
        return static_cast<int>(dir);
    }

    [[nodiscard]] static constexpr Direction fromId(int id)
    {
        return static_cast<Direction>(id);
    }

    [[nodiscard]] static constexpr Direction rotatedClockwise(Direction dir)
    {
        return fromId((toId(dir) + 1) % 4);
    }

    [[nodiscard]] static constexpr Direction rotatedCounterClockwise(Direction dir)
    {
        return fromId((toId(dir) + 3) % 4);
    }

    [[nodiscard]] static constexpr Direction oppositeTo(Direction dir)
    {
        return fromId((toId(dir) + 2) % 4);
    }

    [[nodiscard]] static constexpr bool areOpposite(Direction d1, Direction d2)
    {
        const int diff = toId(d1) - toId(d2);
        return util::abs(diff) == 2;
    }

    [[nodiscard]] static constexpr bool areParallel(Direction d1, Direction d2)
    {
        const int diff = toId(d1) - toId(d2);
        return diff == 0 || util::abs(diff) == 2;
    }

    [[nodiscard]] static constexpr bool arePerpendicular(Direction d1, Direction d2)
    {
        const int diff = toId(d1) - toId(d2);
        return util::abs(diff) == 1 || util::abs(diff) == 3;
    }

    [[nodiscard]] static Coords2i offset(Direction dir)
    {
        static const std::array<Coords2i, 4> offsets{
            Coords2i(0, -1),
            Coords2i(1, 0),
            Coords2i(0, 1),
            Coords2i(-1, 0)
        };

        return offsets[toId(dir)];
    }

    [[nodiscard]] static const std::array<Direction, 4>& values()
    {
        static const std::array<Direction, 4> v{
            Direction::North,
            Direction::East,
            Direction::South,
            Direction::West,
        };

        return v;
    }

    [[nodiscard]] static const std::string& toString(Direction dir)
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

    template <typename... FwdT>
    [[nodiscard]] static constexpr ByDirection<T> nesw(FwdT&& ... args)
    {
        return ByDirection(std::forward<FwdT>(args)...);
    }

    constexpr ByDirection() noexcept = default;
    constexpr ByDirection(const ByDirection&) noexcept = default;
    constexpr ByDirection(ByDirection&&) noexcept = default;
    constexpr ByDirection& operator=(const ByDirection&) noexcept = default;
    constexpr ByDirection& operator=(ByDirection&&) noexcept = default;

    [[nodiscard]] constexpr T& operator[](Direction dir)
    {
        return BaseType::operator[](DirectionHelper::toId(dir));
    }

    [[nodiscard]] constexpr const T& operator[](Direction dir) const
    {
        return BaseType::operator[](DirectionHelper::toId(dir));
    }

private:
    template <typename... FwdT>
    constexpr ByDirection(FwdT&& ... args) :
        BaseType{ std::forward<FwdT>(args)... }
    {

    }
};
