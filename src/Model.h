#pragma once

#include <array>
#include <map>
#include <vector>
#include <utility>
#include <iterator>
#include <algorithm>
#include <random>
#include <future>

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
    using WaveSeedType = std::uint64_t;
    using ModelSeedType = std::uint64_t;

    Model(const Model&) = delete;
    Model(Model&&) = default;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&) = default;
    ~Model() = default;

    std::optional<Array2<CellType>> next()
    {
        return next(m_rng());
    }

    std::optional<Array2<CellType>> next(WaveSeedType seed)
    {
        Wave wave(m_compatibile, m_rng(), this->waveSize(), m_patterns, this->outputWrapping());

        std::vector<float> ps; // preallocate for observeOnce
        ps.resize(m_patterns.size());

        if (!m_firstRun)
        {
            wave.reset();
        }
        m_firstRun = false;

        for (;;)
        {
            switch (wave.observeOnce(ps))
            {
            case Wave::ObservationResult::Contradiction:
                return std::nullopt;
            case Wave::ObservationResult::Finished:
                return this->decodeOutput(std::move(wave));
            default:
                continue;
            }
        }
    }

    // does `parallelism` waves in parallel and returns successful tries
    // so may return less elements than `parallelism`.
    // uses std::async for thread scheduling
    std::vector<Array2<CellType>> nextParallel(int parallelism)
    {
        std::vector<std::future<std::optional<Array2<CellType>>>> futures;
        for (int i = 0; i < parallelism; ++i)
        {
            futures.emplace_back(std::async(std::launch::async, [seed = m_rng(), this]() { return next(seed); }));
        }
        
        std::vector<Array2<CellType>> results;
        for (auto&& future : futures)
        {
            std::optional<Array2<CellType>> result = future.get();
            if (result.has_value())
            {
                results.emplace_back(std::move(result.value()));
            }
        }
        return results;
    }

    // TODO: parallel version that returns exactly n results
    //       and uses a single (lock free) queue to schedule work

    const Patterns<CellType>& patterns() const
    {
        return m_patterns;
    }

    const CompatibilityArrayType& compatibility() const
    {
        return m_compatibile;
    }

protected:
    Model(Patterns<CellType>&& patterns, CompatibilityArrayType&& compatibility, ModelSeedType seed) :
        m_compatibile(std::move(compatibility)),
        m_patterns(std::move(patterns)),
        m_rng(seed),
        m_firstRun(true)
    {
    }

    [[nodiscard]] virtual Array2<CellType> decodeOutput(Wave&& wave) const = 0;
    [[nodiscard]] virtual Size2i waveSize() const = 0;
    [[nodiscard]] virtual WrappingMode outputWrapping() const = 0;

private:
    // m_compatibility[elementId][dir] contains all elements that
    // can be placed next to element with id `elementId` in the `dir` direction
    CompatibilityArrayType m_compatibile;

    Patterns<CellType> m_patterns;

    std::mt19937_64 m_rng;

    bool m_firstRun;
};