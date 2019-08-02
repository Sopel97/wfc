#pragma once

#include <array>
#include <map>
#include <vector>
#include <utility>
#include <iterator>
#include <algorithm>

#include "Array2.h"
#include "NormalizedHistogram.h"
#include "Size2.h"
#include "SmallVector.h"
#include "D4Symmetry.h"
#include "WrappingMode.h"
#include "Logger.h"
#include "Wave.h"
#include "Direction.h"

template <typename CellTypeT>
struct Model
{
    using CellType = CellTypeT;
    using PatternsEntryType = std::pair<typename Patterns<CellType>::ElementType, float>;
    using CompatibilityArrayType = typename Wave::CompatibilityArrayType;

    Model(const Model&) = delete;
    Model(Model&&) = default;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&) = default;
    ~Model() = default;

    std::optional<Array2<CellType>> next()
    {
        std::vector<float> ps; // preallocate for observeOnce
        ps.resize(m_patterns.size());

        if (!m_firstRun)
        {
            m_wave.reset();
        }
        m_firstRun = false;

        for (;;)
        {
            switch (m_wave.observeOnce(ps))
            {
            case Wave::ObservationResult::Contradiction:
                return std::nullopt;
            case Wave::ObservationResult::Finished:
                return this->decodeOutput();
            default:
                continue;
            }
        }
    }

    const Patterns<CellType>& patterns() const
    {
        return m_patterns;
    }

    const CompatibilityArrayType& compatibility() const
    {
        return m_compatibile;
    }

protected:
    Model(Patterns<CellType>&& patterns, CompatibilityArrayType&& compatibility, std::uint64_t seed, Size2i waveSize, WrappingMode waveWrapping) :
        m_compatibile(std::move(compatibility)),
        m_patterns(std::move(patterns)),
        m_wave(m_compatibile, seed, waveSize, m_patterns, waveWrapping),
        m_firstRun(true)
    {
    }

    [[nodiscard]] virtual Array2<CellType> decodeOutput() const = 0;

    const Wave& wave() const
    {
        return m_wave;
    }

private:
    // m_compatibility[elementId][dir] contains all elements that
    // can be placed next to element with id `elementId` in the `dir` direction
    CompatibilityArrayType m_compatibile;

    Patterns<CellType> m_patterns;
    Wave m_wave;
    bool m_firstRun;
};