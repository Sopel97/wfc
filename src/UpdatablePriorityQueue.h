#pragma once

#include <functional>
#include <iterator>
#include <set>
#include <utility>

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
    [[nodiscard]] iterator emplace(ArgsTs&&... args)
    {
        return m_values.emplace(std::forward<ArgsTs>(args)...);
    }

    [[nodiscard]] iterator push(const T& values)
    {
        return m_values.emplace(values);
    }

    [[nodiscard]] iterator push(T&& values)
    {
        return m_values.emplace(std::move(values));
    }

    void erase(iterator iter)
    {
        m_values.erase(iter);
    }

    template <typename FuncT>
    [[nodiscard]] iterator update(iterator iter, FuncT&& func)
    {
        auto node = m_values.extract(iter);
        std::forward<FuncT>(func)(node.value());
        return m_values.insert(std::move(node));
    }

    [[nodiscard]] iterator update(iterator iter, const T& newValue)
    {
        return update(iter, T(newValue));
    }

    [[nodiscard]] iterator update(iterator iter, T&& newValue)
    {
        auto node = m_values.extract(iter);
        node.value() = std::move(newValue);
        return m_values.insert(std::move(node));
    }

    [[nodiscard]] iterator begin()
    {
        return m_values.begin();
    }

    [[nodiscard]] iterator end()
    {
        return m_values.end();
    }

    [[nodiscard]] const_iterator begin() const
    {
        return m_values.begin();
    }

    [[nodiscard]] const_iterator end() const
    {
        return m_values.end();
    }

    [[nodiscard]] const T& top() const
    {
        return *begin();
    }

    void pop()
    {
        erase(begin());
    }

    [[nodiscard]] bool empty() const
    {
        return m_values.empty();
    }

private:
    StorageType m_values;
};
