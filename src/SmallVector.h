#pragma once

template <typename T, int N>
struct SmallVector
{
    static_assert(N > 0 && N < 256, "");

private:
    using StorageType = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

public:

    using value_type = T;
    using size_type = int;
    using difference_type = int;
    using reference = T &;
    using const_reference = const T &;
    using pointer = T *;
    using const_pointer = const T*;
    using iterator = T *;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    SmallVector() :
        m_size(0)
    {
    }

    SmallVector(int s) :
        m_size(s)
    {
        for (int i = 0; i < s; ++i)
        {
            new (&(m_data[i])) T{};
        }
    }

    SmallVector(int s, const T& v) :
        m_size(s)
    {
        for (int i = 0; i < s; ++i)
        {
            new (&(m_data[i])) T(v);
        }
    }

    SmallVector(const SmallVector& other) :
        m_size(other.m_size)
    {
        for (int i = 0; i < m_size; ++i)
        {
            new (&(m_data[i])) T(other[i]);
        }
    }

    SmallVector(SmallVector&& other) noexcept :
        m_size(other.m_size)
    {
        for (int i = 0; i < m_size; ++i)
        {
            new (&(m_data[i])) T(std::move(other[i]));
        }
    }

    ~SmallVector()
    {
        clear();
    }

    SmallVector& operator=(const SmallVector& other)
    {
        clear();

        m_size = other.m_size;
        for (int i = 0; i < m_size; ++i)
        {
            new (&(m_data[i])) T(other[i]);
        }
    }

    SmallVector& operator=(SmallVector&& other) noexcept
    {
        clear();

        m_size = other.m_size;
        for (int i = 0; i < m_size; ++i)
        {
            new (&(m_data[i])) T(std::move(other[i]));
        }
    }

    [[nodiscard]] pointer data()
    {
        return reinterpret_cast<T*>(&(m_data[0]));
    }

    [[nodiscard]] const_pointer data() const
    {
        return reinterpret_cast<const T*>(&(m_data[0]));
    }

    [[nodiscard]] reference operator[](int i)
    {
        return (data()[i]);
    }

    [[nodiscard]] const_reference operator[](int i) const
    {
        return (data()[i]);
    }

    [[nodiscard]] reference front()
    {
        return (*this)[0];
    }

    [[nodiscard]] const_reference front() const
    {
        return (*this)[0];
    }

    [[nodiscard]] reference back()
    {
        return (*this)[m_size - 1];
    }

    [[nodiscard]] const_reference back() const
    {
        return (*this)[m_size - 1];
    }

    [[nodiscard]] iterator begin()
    {
        return data();
    }

    [[nodiscard]] iterator end()
    {
        return data() + m_size;
    }

    [[nodiscard]] const_iterator begin() const
    {
        return data();
    }

    [[nodiscard]] const_iterator end() const
    {
        return data() + m_size;
    }

    [[nodiscard]] const_iterator cbegin() const
    {
        return data();
    }

    [[nodiscard]] const_iterator cend() const
    {
        return data() + m_size;
    }

    [[nodiscard]] reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }

    [[nodiscard]] reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    [[nodiscard]] const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }

    [[nodiscard]] const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

    [[nodiscard]] const_reverse_iterator crbegin() const
    {
        return const_reverse_iterator(cend());
    }

    [[nodiscard]] const_reverse_iterator crend() const
    {
        return const_reverse_iterator(cbegin());
    }

    [[nodiscard]] bool empty() const
    {
        return m_size == 0;
    }

    [[nodiscard]] size_type size() const
    {
        return m_size;
    }

    [[nodiscard]] size_type capacity() const
    {
        return N;
    }

    void clear()
    {
        for (int i = 0; i < m_size; ++i)
        {
            (*this)[i].~T();
        }
        m_size = 0;
    }

    template<typename... ArgsTs>
    reference emplace_back(ArgsTs&& ... args)
    {
        new (&(m_data[m_size])) T(std::forward<ArgsTs>(args)...);
        ++m_size;
        return back();
    }

    reference push_back(const T& value)
    {
        new (&(m_data[m_size])) T(value);
        ++m_size;
        return back();
    }

    reference push_back(T&& value)
    {
        new (&(m_data[m_size])) T(std::move(value));
        ++m_size;
        return back();
    }

    void pop_back()
    {
        --m_size;
        (*this)[m_size].~T();
    }

private:
    size_type m_size;
    StorageType m_data[N];
};
