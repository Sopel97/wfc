#pragma once

#include <array>
#include <map>
#include <vector>
#include <utility>
#include <iterator>
#include <algorithm>

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
        std::vector<float> ps; // preallocate for observeOnce
        ps.resize(m_patterns.size());

        if (!m_firstRun)
        {
            m_wave.reset();
        }
        m_firstRun = false;

        for (;;)
        {
            switch (observeOnce(ps))
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
        m_wave(std::move(compatibility), waveSize, m_patterns, waveWrapping),
        m_firstRun(true)
    {
    }

    [[nodiscard]] virtual Array2<CellType> decodeOutput() const = 0;

    enum struct ObservationResult
    {
        Contradiction,
        Finished,
        Unfinished
    };

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
    bool m_firstRun;

    [[nodiscard]] ObservationResult observeOnce(std::vector<float>& ps) noexcept
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
        float pssum = 0.0f;
        {
            for (int i = 0; i < numPatterns; ++i)
            {
                pssum += m_wave.canBePlaced(pos, i) ? m_patterns.frequency(i) : 0.0f;
                ps[i] = pssum;
            }
        }

        std::uniform_real_distribution<float> dPssum(0.0f, pssum);
        const float r = std::min(dPssum(m_rng), pssum); // min just in case of unfortunate rounding
        const auto iter = std::lower_bound(std::begin(ps), std::end(ps), r);
        const int patternId = static_cast<int>(std::distance(std::begin(ps), iter));

        m_wave.setElement(pos, patternId);

        return ObservationResult::Unfinished;
    }
};