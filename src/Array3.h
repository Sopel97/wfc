#pragma once

#include <algorithm>
#include <memory>

#include "Coords3.h"
#include "Size3.h"

// stored in 'depth-major' order
// ie. A[x][y][0..depth-1] is contiguous
template <typename T>
struct Array3
{
    Array3() :
        m_size(0, 0, 0),
        m_values(nullptr)
    {

    }

    Array3(Size3i size, const T& value = T{}) :
        m_size(size),
        m_values(new T[size.total()])
    {
        std::fill(begin(), end(), value);
    }

    Array3(const Array3& other) :
        m_size(other.m_size),
        m_values(new T[other.m_size.total()])
    {
        std::copy(std::begin(other), std::end(other), begin());
    }

    Array3(Array3&& other) noexcept :
        m_size(other.m_size),
        m_values(std::move(other.m_values))
    {
    }

    Array3& operator=(const Array3& other)
    {
        m_size = other.m_size;
        m_values.reset(new T[m_size.total()]);
        std::copy(std::begin(other), std::end(other), begin());
        return *this;
    }

    Array3& operator=(Array3&& other) noexcept
    {
        m_size = other.m_size;
        m_values = std::move(other.m_values);
        return *this;
    }

    void fill(const T& v)
    {
        std::fill(begin(), end(), v);
    }

    [[nodiscard]] T* begin()
    {
        return m_values.get();
    }

    [[nodiscard]] const T* begin() const
    {
        return m_values.get();
    }

    [[nodiscard]] T* end()
    {
        return m_values.get() + m_size.total();
    }

    [[nodiscard]] const T* end() const
    {
        return m_values.get() + m_size.total();
    }

    [[nodiscard]] T* operator()(int x, int y)
    {
        return m_values.get() + ((x * m_size.height + y) * m_size.depth);
    }

    [[nodiscard]] const T* operator()(int x, int y) const
    {
        return m_values.get() + ((x * m_size.height + y) * m_size.depth);
    }

    [[nodiscard]] T& operator()(int x, int y, int z)
    {
        return m_values[(x * m_size.height + y) * m_size.depth + z];
    }

    [[nodiscard]] const T& operator()(int x, int y, int z) const
    {
        return m_values[(x * m_size.height + y) * m_size.depth + z];
    }

    [[nodiscard]] T* operator[](Coords2i coords)
    {
        return (*this)(coords.x, coords.y);
    }

    [[nodiscard]] const T* operator[](Coords2i coords) const
    {
        return (*this)(coords.x, coords.y);
    }

    [[nodiscard]] T& operator[](Coords3i coords)
    {
        return (*this)(coords.x, coords.y, coords.z);
    }

    [[nodiscard]] const T& operator[](Coords3i coords) const
    {
        return (*this)(coords.x, coords.y, coords.z);
    }

    [[nodiscard]] int getFlatIndex(Coords3i coords) const
    {
        return (coords.x * m_size.height + coords.y) * m_size.depth + coords.z;
    }

    [[nodiscard]] const T* data() const
    {
        return m_values.get();
    }

    [[nodiscard]] T* data()
    {
        return m_values.get();
    }

    [[nodiscard]] Size3i size() const
    {
        return m_size;
    }

private:
    Size3i m_size;
    std::unique_ptr<T[]> m_values;
};

template <typename T, typename Func>
void forEach(Array3<T>& a, Func&& func)
{
    const auto [width, height, depth] = a.size();
    auto* data = a.data();
    for (int x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; ++y)
        {
            for (int z = 0; z < depth; ++z)
            {
                func(data[(x * height + y) * depth + z], x, y, z);
            }
        }
    }
}

template <typename T, typename Func>
void forEach(const Array3<T>& a, Func&& func)
{
    const auto [width, height, depth] = a.size();
    const auto* data = a.data();
    for (int x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; ++y)
        {
            for (int z = 0; z < depth; ++z)
            {
                func(data[(x * height + y) * depth + z], x, y, z);
            }
        }
    }
}
