#pragma once

#include <algorithm>
#include <memory>

#include "Coords2.h"
#include "Size2.h"
#include "SmallVector.h"
#include "D4Symmetry.h"
#include "WrappingMode.h"

template <typename T>
struct Array2;

// stored in column-major order
// ie. A[x][0..height-1] is contiguous
template <typename T>
struct SquareArray2
{
    friend struct Array2<T>;

    SquareArray2() :
        m_size(0),
        m_values(nullptr)
    {

    }

    SquareArray2(int size, const T& value = T{}) :
        m_size(size),
        m_values(new T[size * size])
    {
        std::fill(begin(), end(), value);
    }

    SquareArray2(const SquareArray2& other) :
        m_size(other.m_size),
        m_values(new T[other.m_size * other.m_size])
    {
        std::copy(std::begin(other), std::end(other), begin());
    }

    SquareArray2(SquareArray2&& other) noexcept :
        m_size(other.m_size),
        m_values(std::move(other.m_values))
    {
    }

    SquareArray2& operator=(const SquareArray2& other)
    {
        m_size = other.m_size;
        m_values.reset(new T[m_size * size]);
        std::copy(std::begin(other), std::end(other), begin());
        return *this;
    }

    SquareArray2& operator=(SquareArray2&& other) noexcept
    {
        m_size = other.m_size;
        m_values = std::move(other.m_values);
        return *this;
    }

    void fill(const T& v)
    {
        std::fill(begin(), end(), v);
    }

    template <typename FuncT>
    [[nodiscard]] SquareArray2<T> transformed(FuncT&& f) const
    {
        SquareArray2<T> res(m_size);

        for (int x = 0; x < m_size; ++x)
        {
            for (int y = 0; y < m_size; ++y)
            {
                const Coords2i xy(x, y);
                const Coords2i txy = f(xy, m_size);
                res[xy] = (*this)[txy];
            }
        }

        return res;
    }

    [[nodiscard]] SquareArray2<T> rotated90() const
    {
        // A B      B D
        // C D  ->  A C

        return transformed([](Coords2i c, int size) { return Coords2i(size - c.y - 1, c.x); });
    }

    [[nodiscard]] SquareArray2<T> rotated180() const
    {
        // A B      D C
        // C D  ->  B A

        return transformed([](Coords2i c, int size) { return Coords2i(size - c.x - 1, size - c.y - 1); });
    }

    [[nodiscard]] SquareArray2<T> rotated270() const
    {
        // A B      C A
        // C D  ->  D B

        return transformed([](Coords2i c, int size) { return Coords2i(c.y, size - c.x - 1); });
    }

    [[nodiscard]] SquareArray2<T> flippedAboutHorizontalAxis() const
    {
        // A B      C D
        // C D  ->  A B

        return transformed([](Coords2i c, int size) { return Coords2i(c.x, size - c.y - 1); });
    }

    [[nodiscard]] SquareArray2<T> flippedAboutVerticalAxis() const
    {
        // A B      B A
        // C D  ->  D C

        return transformed([](Coords2i c, int size) { return Coords2i(size - c.x - 1, c.y); });
    }

    [[nodiscard]] SquareArray2<T> flippedAboutMainDiagonal() const
    {
        // A B      A C
        // C D  ->  B D

        return transformed([](Coords2i c, int size) { return Coords2i(c.y, c.x); });
    }

    [[nodiscard]] SquareArray2<T> flippedAboutAntiDiagonal() const
    {
        // A B      D B
        // C D  ->  C A

        return transformed([](Coords2i c, int size) { return Coords2i(size - c.y - 1, size - c.x - 1); });
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
        return m_values.get() + m_size * m_size;
    }

    [[nodiscard]] const T* end() const
    {
        return m_values.get() + m_size * m_size;
    }

    [[nodiscard]] T* operator[](int x)
    {
        return begin() + x * m_size;
    }

    [[nodiscard]] const T* operator[](int x) const
    {
        return begin() + x * m_size;
    }

    [[nodiscard]] T& operator[](Coords2i c)
    {
        return (*this)[c.x][c.y];
    }

    [[nodiscard]] const T& operator[](Coords2i c) const
    {
        return (*this)[c.x][c.y];
    }

    [[nodiscard]] int size() const
    {
        return m_size;
    }

    [[nodiscard]] friend bool operator<(const SquareArray2<T>& lhs, const SquareArray2<T>& rhs) noexcept
    {
        if (lhs.m_size != rhs.m_size)
        {
            return lhs.m_size < rhs.m_size;
        }

        const int totalSize = lhs.m_size * lhs.m_size;
        for (int i = 0; i < totalSize; ++i)
        {
            if (lhs.m_values[i] != rhs.m_values[i])
            {
                return lhs.m_values[i] < rhs.m_values[i];
            }
        }

        return false;
    }

private:
    int m_size;
    std::unique_ptr<T[]> m_values;

    explicit SquareArray2(int size, std::unique_ptr<T[]>&& values) :
        m_size(size),
        m_values(std::move(values))
    {
    }
};

template <typename T, typename Func>
void forEach(SquareArray2<T>& a, Func&& func)
{
    const auto size = a.size();
    for (int x = 0; x < size; ++x)
    {
        for (int y = 0; y < size; ++y)
        {
            func(a[x][y], x, y);
        }
    }
}

template <typename T, typename Func>
void forEach(const SquareArray2<T>& a, Func&& func)
{
    const auto size = a.size();
    for (int x = 0; x < size; ++x)
    {
        for (int y = 0; y < size; ++y)
        {
            func(a[x][y], x, y);
        }
    }
}



// stored in column-major order
// ie. A[x][0..height-1] is contiguous
template <typename T>
struct Array2
{
    Array2() :
        m_size(0, 0),
        m_values(nullptr)
    {

    }

    Array2(Size2i size, const T& value = T{}) :
        m_size(size),
        m_values(new T[size.total()])
    {
        std::fill(begin(), end(), value);
    }

    Array2(const Array2& other) :
        m_size(other.m_size),
        m_values(new T[other.m_size.total()])
    {
        std::copy(std::begin(other), std::end(other), begin());
    }

    Array2(Array2&& other) noexcept :
        m_size(other.m_size),
        m_values(std::move(other.m_values))
    {
    }

    Array2(SquareArray2<T>&& other) noexcept :
        m_size(other.m_size, other.m_size),
        m_values(std::move(other.m_values))
    {
    }

    Array2& operator=(const Array2& other)
    {
        m_size = other.m_size;
        m_values.reset(new T[m_size.total()]);
        std::copy(std::begin(other), std::end(other), begin());
        return *this;
    }

    Array2& operator=(Array2&& other) noexcept
    {
        m_size = other.m_size;
        m_values = std::move(other.m_values);
        return *this;
    }

    Array2& operator=(SquareArray2<T>&& other) noexcept
    {
        m_size = { other.m_size, other.m_size };
        m_values = std::move(other.m_values);
        return *this;
    }

    void fill(const T& v)
    {
        std::fill(begin(), end(), v);
    }

    template <WrappingMode WrapV>
    [[nodiscard]] Array2<T> sub(Coords2i topLeft, Size2i newSize) const
    {
        Array2<T> res(newSize);

        for (int xx = 0; xx < newSize.width; ++xx)
        {
            for (int yy = 0; yy < newSize.height; ++yy)
            {
                int x = topLeft.x + xx;
                int y = topLeft.y + yy;
                if constexpr (contains(WrapV, WrappingMode::Horizontal))
                {
                    x %= m_size.width;
                }
                if constexpr (contains(WrapV, WrappingMode::Vertical))
                {
                    y %= m_size.height;
                }

                res[xx][yy] = (*this)[x][y];
            }
        }

        return res;
    }

    [[nodiscard]] Array2<T> sub(Coords2i topLeft, Size2i newSize, WrappingMode wrap) const
    {
        switch (wrap)
        {
        case WrappingMode::None:
            return sub<WrappingMode::None>(topLeft, newSize);
        case WrappingMode::Horizontal:
            return sub<WrappingMode::Horizontal>(topLeft, newSize);
        case WrappingMode::Vertical:
            return sub<WrappingMode::Vertical>(topLeft, newSize);
        case WrappingMode::All:
            return sub<WrappingMode::All>(topLeft, newSize);
        }
    }

    template <WrappingMode WrapV>
    [[nodiscard]] SquareArray2<T> sub(Coords2i topLeft, int newSize) const
    {
        SquareArray2<T> res(newSize);

        for (int xx = 0; xx < newSize; ++xx)
        {
            for (int yy = 0; yy < newSize; ++yy)
            {
                int x = topLeft.x + xx;
                int y = topLeft.y + yy;
                if constexpr (contains(WrapV, WrappingMode::Horizontal))
                {
                    x %= m_size.width;
                }
                if constexpr (contains(WrapV, WrappingMode::Vertical))
                {
                    y %= m_size.height;
                }

                res[xx][yy] = (*this)[x][y];
            }
        }

        return res;
    }

    [[nodiscard]] SquareArray2<T> sub(Coords2i topLeft, int newSize, WrappingMode wrap) const
    {
        switch (wrap)
        {
        case WrappingMode::None:
            return sub<WrappingMode::None>(topLeft, newSize);
        case WrappingMode::Horizontal:
            return sub<WrappingMode::Horizontal>(topLeft, newSize);
        case WrappingMode::Vertical:
            return sub<WrappingMode::Vertical>(topLeft, newSize);
        case WrappingMode::All:
            return sub<WrappingMode::All>(topLeft, newSize);
        }
    }

    SquareArray2<T> square() const &
    {
        return sub<WrappingMode::None>({ 0, 0 }, std::min(m_size.width, m_size.height));
    }

    SquareArray2<T> square() &&
    {
        if (m_size.width == m_size.height)
        {
            return SquareArray2<T>(m_size.width, std::move(m_values));
        }
        else
        {
            return sub<WrappingMode::None>({ 0, 0 }, std::min(m_size.width, m_size.height));
        }
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

    [[nodiscard]] T* operator[](int x)
    {
        return begin() + x * m_size.height;
    }

    [[nodiscard]] const T* operator[](int x) const
    {
        return begin() + x * m_size.height;
    }

    [[nodiscard]] T& operator[](Coords2i c)
    {
        return (*this)[c.x][c.y];
    }

    [[nodiscard]] const T& operator[](Coords2i c) const
    {
        return (*this)[c.x][c.y];
    }

    [[nodiscard]] Size2i size() const
    {
        return m_size;
    }

private:
    Size2i m_size;
    std::unique_ptr<T[]> m_values;
};

template <typename T, typename Func>
void forEach(Array2<T>& a, Func&& func)
{
    const auto [width, height] = a.size();
    for (int x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; ++y)
        {
            func(a[x][y], x, y);
        }
    }
}

template <typename T, typename Func>
void forEach(const Array2<T>& a, Func&& func)
{
    const auto [width, height] = a.size();
    for (int x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; ++y)
        {
            func(a[x][y], x, y);
        }
    }
}

template <typename T>
[[nodiscard]] SmallVector<SquareArray2<T>, 8> generateSymmetries(SquareArray2<T>&& pattern, D4Symmetries symmetries)
{
    SmallVector<SquareArray2<T>, 8> sym;

    if (contains(symmetries, D4Symmetry::Rotation90))
    {
        sym.emplace_back(pattern.rotated90());
    }
    if (contains(symmetries, D4Symmetry::Rotation180))
    {
        sym.emplace_back(pattern.rotated180());
    }
    if (contains(symmetries, D4Symmetry::Rotation270))
    {
        sym.emplace_back(pattern.rotated270());
    }
    if (contains(symmetries, D4Symmetry::FlipAboutHorizontalAxis))
    {
        sym.emplace_back(pattern.flippedAboutHorizontalAxis());
    }
    if (contains(symmetries, D4Symmetry::FlipAboutVerticalAxis))
    {
        sym.emplace_back(pattern.flippedAboutVerticalAxis());
    }
    if (contains(symmetries, D4Symmetry::FlipAboutMainDiagonal))
    {
        sym.emplace_back(pattern.flippedAboutMainDiagonal());
    }
    if (contains(symmetries, D4Symmetry::FlipAboutAntiDiagonal))
    {
        sym.emplace_back(pattern.flippedAboutAntiDiagonal());
    }

    sym.emplace_back(std::move(pattern));

    return sym;
}

// returns true if overlapping elements of (`lhs`) and (`rhs` offset by `offset`) are equal
template <typename T>
[[nodiscard]] bool overlapEqualWhenOffset(const SquareArray2<T>& lhs, const SquareArray2<T>& rhs, Coords2i offset)
{
    const auto [dx, dy] = offset;
    const auto lhsSize = lhs.size();
    const auto rhsSize = rhs.size();

    // compute intersection
    // in lhs's coordinates
    const int xbegin = std::max(0, dx);
    const int xend = std::min(lhsSize, rhsSize + dx);
    const int ybegin = std::max(0, dy);
    const int yend = std::min(lhsSize, rhsSize + dy);

    for (int x = xbegin; x < xend; ++x)
    {
        for (int y = ybegin; y < yend; ++y)
        {
            if (lhs[x][y] != rhs[x - dx][y - dy])
            {
                return false;
            }
        }
    }

    return true;
}
