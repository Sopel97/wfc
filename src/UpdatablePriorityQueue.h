#pragma once

#include <set>

template <typename T, typename CompT = std::less<>>
struct UpdatablePriorityQueue
{
    using StorageType = std::multiset<T, CompT>;

    using iterator = typename StorageType::iterator;
    using const_iterator = typename StorageType::const_iterator;

    UpdatablePriorityQueue(CompT cmp = {}) :
        m_values(cmp)
    {
    }

    template <typename... ArgsTs>
    iterator emplace(ArgsTs&&... args)
    {
        return m_values.emplace(std::forward<ArgsTs>(args)...);
    }

    iterator push(const T& values)
    {
        return m_values.emplace(values);
    }

    iterator push(T&& values)
    {
        return m_values.emplace(std::move(values));
    }

    void erase(iterator iter)
    {
        m_values.erase(iter);
    }

    template <typename FuncT>
    iterator update(iterator iter, FuncT&& func)
    {
        auto node = m_values.extract(iter);
        std::forward<FuncT>(func)(node.value());
        return m_values.insert(std::move(node));
    }

    iterator update(iterator iter, const T& newValue)
    {
        return update(iter, T(newValue));
    }

    iterator update(iterator iter, T&& newValue)
    {
        auto node = m_values.extract(iter);
        node.value() = std::move(newValue);
        return m_values.insert(std::move(node));
    }

    iterator begin()
    {
        return m_values.begin();
    }

    iterator end()
    {
        return m_values.end();
    }

    const_iterator begin() const
    {
        return m_values.begin();
    }

    const_iterator end() const
    {
        return m_values.end();
    }

    const T& top() const
    {
        return *begin();
    }

    void pop()
    {
        erase(begin());
    }

    bool empty() const
    {
        return m_values.empty();
    }

private:
    StorageType m_values;
};
