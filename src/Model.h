#pragma once

#include <array>
#include <map>
#include <vector>
#include <utility>

#include "lib/pcg_random.hpp"

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
    using RandomNumberGeneratorType = pcg32_fast;
    using PatternsEntryType = std::pair<typename Patterns<CellType>::ElementType, float>;

    std::optional<Array2<CellType>> next()
    {
        m_wave.reset();
        for (;;)
        {
            switch (observeOnce())
            {
            case ObservationResult::Contradiction:
                return std::nullopt;
            case ObservationResult::Finished:
                return this->decodeOutput();
            default:
                continue;
            }
        }
    }

protected:
    Model(RandomNumberGeneratorType&& rng, Patterns<CellType>&& patterns, Wave::CompatibilityArrayType&& compatibility, Size2i waveSize, WrappingMode waveWrapping) :
        m_rng(std::move(rng)),
        m_patterns(std::move(patterns)),
        m_wave(std::move(compatibility), waveSize, m_patterns, waveWrapping)
    {
    }

    [[nodiscard]] virtual Array2<CellType> decodeOutput() const = 0;

    enum struct ObservationResult
    {
        Contradiction,
        Finished,
        Unfinished
    };

    [[nodiscard]] ObservationResult observeOnce() noexcept
    {
        const auto [status, pos] = m_wave.posWithMinimalEntropy(m_rng);

        if (status == Wave::MinimalEntropyQueryResult::Contradiction)
        {
            return ObservationResult::Contradiction;
        }

        if (status == Wave::MinimalEntropyQueryResult::Finished)
        {
            return ObservationResult::Finished;
        }

        LOG_DEBUG(g_logger, "Observed (", pos.x, ", ", pos.y, ")");

        const int numPatterns = m_patterns.size();

        // choose an element according to the pattern distribution
        std::vector<float> ps;
        ps.reserve(numPatterns);
        {
            for (int i = 0; i < numPatterns; ++i)
            {
                ps.emplace_back(m_wave.canBePlaced(pos, i) ? m_patterns.frequency(i) : 0.0f);
            }
        }

        std::discrete_distribution<int> dPatternId(std::begin(ps), std::end(ps));
        const int patternId = dPatternId(m_rng);

        // define the cell with the chosen pattern by disabling others
        for (int i = 0; i < numPatterns; ++i)
        {
            if (i == patternId)
            {
                continue;
            }

            m_wave.makeUnplacable(pos, i);
        }

        m_wave.propagate();

        return ObservationResult::Unfinished;
    }

    const Wave& wave() const
    {
        return m_wave;
    }

    const Patterns<CellType>& patterns() const
    {
        return m_patterns;
    }

private:
    RandomNumberGeneratorType m_rng;
    Patterns<CellType> m_patterns;
    Wave m_wave;
};