#pragma once

#include <iterator>

template <typename IterT>
struct IterSpan
{
    IterSpan(IterT begin, IterT end) :
        m_begin(begin),
        m_end(end)
    {
    }

    template <typename ContainerT>
    IterSpan(ContainerT&& cont) :
        m_begin(std::begin(cont)),
        m_end(std::end(cont))
    {
    }

    IterT begin() const
    {
        return m_begin;
    }

    IterT end() const
    {
        return m_end;
    }

    int size() const
    {
        return static_cast<int>(std::distance(m_begin, m_end));
    }

    decltype(auto) operator[](int i)
    {
        return *(m_begin + i);
    }

    decltype(auto) operator[](int i) const
    {
        return *(m_begin + i);
    }

private:
    IterT m_begin;
    IterT m_end;
};
