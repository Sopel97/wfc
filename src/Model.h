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

template <typename CellTypeT>
struct Model
{
    using CellType = CellTypeT;
    using PatternsEntryType = std::pair<typename Patterns<CellType>::ElementType, float>;

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

protected:
    Model(Patterns<CellType>&& patterns, Wave::CompatibilityArrayType&& compatibility, std::uint64_t seed, Size2i waveSize, WrappingMode waveWrapping) :
        m_patterns(std::move(patterns)),
        m_wave(std::move(compatibility), seed, waveSize, m_patterns, waveWrapping),
        m_firstRun(true)
    {
    }

    [[nodiscard]] virtual Array2<CellType> decodeOutput() const = 0;

    const Wave& wave() const
    {
        return m_wave;
    }

    const Patterns<CellType>& patterns() const
    {
        return m_patterns;
    }

private:
    Patterns<CellType> m_patterns;
    Wave m_wave;
    bool m_firstRun;
};