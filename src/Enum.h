#pragma once

#include <array>

template <typename EnumT>
struct EnumTraits;

template <typename EnumT>
[[nodiscard]] constexpr int cardinality() noexcept
{
    return EnumTraits<EnumT>::cardinality;
}

template <typename EnumT>
[[nodiscard]] constexpr std::array<EnumT, cardinality<EnumT>()> values() noexcept
{
    return EnumTraits<EnumT>::values;
}

template <typename EnumT>
[[nodiscard]] constexpr EnumT fromId(int id) noexcept
{
    return EnumTraits<EnumT>::fromId(id);
}

template <typename EnumT>
[[nodiscard]] constexpr typename EnumTraits<EnumT>::IdType toId(EnumT v) noexcept
{
    return EnumTraits<EnumT>::toId(v);
}
