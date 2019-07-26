#pragma once

#include <iterator>
#include <vector>

#include "Array2.h"

struct NormalizedFrequencies
{
    using FrequencyType = float;
    using FrequenciesType = std::vector<FrequencyType>;

    NormalizedFrequencies() = default;
    NormalizedFrequencies(const NormalizedFrequencies&) = default;
    NormalizedFrequencies(NormalizedFrequencies&&) = default;
    NormalizedFrequencies& operator=(const NormalizedFrequencies&) = default;
    NormalizedFrequencies& operator=(NormalizedFrequencies&&) = default;
    ~NormalizedFrequencies() = default;

    const FrequenciesType& frequencies() const
    {
        return m_frequencies;
    }

    const FrequencyType& frequency(int i) const
    {
        return m_frequencies[i];
    }

    int size() const
    {
        return static_cast<int>(m_frequencies.size());
    }

    void reserve(int n)
    {
        m_frequencies.reserve(n);
    }

protected:
    FrequenciesType m_frequencies;
};

template <typename ElementT>
struct NormalizedHistogram : public NormalizedFrequencies
{
    using ElementType = ElementT;
    using ElementsType = std::vector<ElementType>;

    NormalizedHistogram() = default;
    NormalizedHistogram(const NormalizedHistogram&) = default;
    NormalizedHistogram(NormalizedHistogram&&) = default;
    NormalizedHistogram& operator=(const NormalizedHistogram&) = default;
    NormalizedHistogram& operator=(NormalizedHistogram&&) = default;
    ~NormalizedHistogram() = default;

    // moves from the values pointed to by iterators
    template <typename IterT>
    NormalizedHistogram(IterT begin, IterT end)
    {
        const int size = static_cast<int>(std::distance(begin, end));
        reserve(size);

        float total = 0.0f;
        while (begin != end)
        {
            auto&& [element, count] = *begin;
            m_elements.emplace_back(std::move(element));
            m_frequencies.emplace_back(count);
            total += count;
            ++begin;
        }

        const float invTotal = 1.0f / total;
        for (auto& f : m_frequencies)
        {
            f *= invTotal;
        }
    }

    const ElementType& element(int i) const
    {
        return m_elements[i];
    }

    const ElementsType& elements() const
    {
        return m_elements;
    }

    void reserve(int n)
    {
        m_frequencies.reserve(n);
        m_elements.reserve(n);
    }

private:
    ElementsType m_elements;
};

template <typename CellType>
using Patterns = NormalizedHistogram<SquareArray2<CellType>>;
