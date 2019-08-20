#pragma once

#include <algorithm>
#include <array>
#include <future>
#include <iterator>
#include <map>
#include <random>
#include <utility>
#include <vector>

#include "Array2.h"
#include "D4Symmetry.h"
#include "Direction.h"
#include "Logger.h"
#include "NormalizedHistogram.h"
#include "Size2.h"
#include "SmallVector.h"
#include "Wave.h"
#include "WrappingMode.h"

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
    virtual ~Model() = default;

    [[nodiscard]] virtual std::optional<Array2<CellType>> next()
    {
        return next(m_rng());
    }

    [[nodiscard]] virtual std::optional<Array2<CellType>> next(WaveSeedType seed)
    {
        Wave wave(m_compatibile, seed, this->waveSize(), m_patterns, this->outputWrapping());

        std::vector<float> ps; // preallocate for observeOnce
        ps.resize(m_patterns.size());

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

    // does `tries` waves (in parallel) and returns successful tries
    // so may return less elements than `tries`.
    // uses std::async for thread scheduling
    [[nodiscard]] virtual std::vector<Array2<CellType>> tryNextN(std::execution::parallel_policy, int tries)
    {
        std::vector<std::future<std::optional<Array2<CellType>>>> futures;
        for (int i = 0; i < tries; ++i)
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

    [[nodiscard]] virtual const Patterns<CellType>& patterns() const final
    {
        return m_patterns;
    }

    [[nodiscard]] virtual const CompatibilityArrayType& compatibility() const final
    {
        return m_compatibile;
    }

protected:
    Model(Patterns<CellType>&& patterns, CompatibilityArrayType&& compatibility, ModelSeedType seed) :
        m_compatibile(std::move(compatibility)),
        m_patterns(std::move(patterns)),
        m_rng(seed)
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
};