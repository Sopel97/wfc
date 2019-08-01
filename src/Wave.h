#pragma once

#include <algorithm>
#include <cmath>
#include <iterator>
#include <optional>
#include <queue>
#include <random>
#include <execution>

#include "lib/pcg_random.hpp"

#include "Algorithm.h"
#include "Array2.h"
#include "Array3.h"
#include "Direction.h"
#include "NormalizedHistogram.h"
#include "UpdatablePriorityQueue.h"
#include "Logger.h"
#include "Util.h"

struct Wave
{
    using CompatibilityArrayType = std::vector<ByDirection<std::vector<int>>>;
    using RandomNumberGeneratorType = pcg32_fast;

private:
    struct EntropyQueueEntry
    {
        float entropy;

        int index;

        [[nodiscard]] friend bool operator<(EntropyQueueEntry lhs, EntropyQueueEntry rhs) noexcept
        {
            return lhs.entropy < rhs.entropy;
        }
    };
    using EntropyQueueType = UpdatablePriorityQueue<EntropyQueueEntry>;
    using EntropyQueueIterator = typename EntropyQueueType::iterator;

    struct Entry
    {
        // sum { p'(element) * log(p'(element)) }
        float plogpSum;

        // sum { p'(element) }
        float pSum;

        int numAvailableElements;

        float entropy;

        EntropyQueueIterator iter = {};
    };

    static_assert(std::decay_t<RandomNumberGeneratorType>::min() == 0);
    static constexpr float rngMax = static_cast<float>(std::decay_t<RandomNumberGeneratorType>::max());

    RandomNumberGeneratorType m_rng;

    Size2i m_size;

    // min { (p * log(p)) / 2 }
    float m_noiseMax;

    WrappingMode m_wrapping;

    bool m_hasContradiction;

    // p
    const float* m_p;

    // p * log(p)
    std::vector<float> m_plogp;

    Entry m_initEntry;

    Array2<Entry> m_memo;

    // m_canBePlaced[x][y][elementId]
    Array3<bool> m_canBePlaced;

    // m_compatibility[elementId][dir] contains all elements that
    // can be placed next to element with id `elementId` in the `dir` direction
    CompatibilityArrayType m_compatibile;

    // m_numCompatibile[{x, y, elementId}][dir]
    // denotes the number of elements in the wave that can be placed
    // at ((x, y) + opposite(dir)) without contradiction with (x, y)
    // if m_canBePlaced[x][y][elementId] == false then
    // m_numCompatibile[{x, y, elementId}][dir] <= 0 for every dir
    Array3<ByDirection<int>> m_numCompatibile;

    // each elements holds {x, y, elementId}
    std::vector<Coords3i> m_propagationQueue;

    EntropyQueueType m_entropyQueue;

    Array3<ByDirection<int>> initNumCompatibile() const
    {
        const auto [width, height] = size();
        const int numElements = static_cast<int>(m_plogp.size());

        Array3<ByDirection<int>> res({ width, height, numElements });

        forEach(res, [&](ByDirection<int>& r, int x, int y, int elementId) {
            const auto& compatibile = m_compatibile[elementId];

            ByDirection<int> counts{};

            for (Direction dir : values<Direction>())
            {
                counts[dir] = static_cast<int>(compatibile[oppositeTo(dir)].size());
            }

            r = counts;
        });

        return res;
    }

    auto randomNoiseGenerator(float max)
    {
        auto dNoise = [scale = max * (1.0f / rngMax), this]() {
            return m_rng() * scale;
        };
        return dNoise;
    }

public:
    enum struct MinimalEntropyQueryResult
    {
        Success,
        Contradiction,
        Finished
    };

    enum struct ObservationResult
    {
        Contradiction,
        Finished,
        Unfinished
    };

    Wave(CompatibilityArrayType&& compatibility, std::uint64_t seed, Size2i size, const NormalizedFrequencies& freq, WrappingMode wrapping) :
        m_rng(seed),
        m_size(size),
        m_noiseMax(std::numeric_limits<float>::max()),
        m_wrapping(wrapping),
        m_hasContradiction(false),
        m_p(freq.frequencies().data()),
        m_plogp(freq.size()),
        m_memo(size),
        m_canBePlaced(Size3i(size, freq.size()), true),
        m_compatibile(std::move(compatibility)),
        m_numCompatibile(initNumCompatibile())
    {
        std::transform(
            m_p,
            m_p + freq.size(),
            std::begin(m_plogp),
            [](float p) { return p * util::approximateLog(p); }
        );

        for (auto&& plogp : m_plogp)
        {
            m_noiseMax = std::min(m_noiseMax, std::abs(plogp));
        }
        m_noiseMax *= 0.5f;

        // frequencies are normalized so
        // base_s = 1, log(base_s) = 0
        // simplifies the equation

        float baseEntropy = 0;
        for (int i = 0; i < freq.size(); ++i) {
            baseEntropy += m_plogp[i];
        }

        m_initEntry = Entry{ baseEntropy, 1.0f, static_cast<int>(freq.size()), -baseEntropy };
        auto dNoise = randomNoiseGenerator(m_noiseMax);
        for (auto& e : m_memo)
        {
            e.entropy += dNoise();
        }

        std::fill(std::begin(m_memo), std::end(m_memo), m_initEntry);

        LOG_INFO(g_logger, "Created wave");
        LOG_INFO(g_logger, "baseEntropy = ", baseEntropy);
        LOG_INFO(g_logger, "numAvailableElements = ", freq.size());
        LOG_INFO(g_logger, "entropy = ", -baseEntropy);
        LOG_INFO(g_logger, "noiseMax = ", m_noiseMax);
        LOG_INFO(g_logger, "size = (", m_size.width, ", ", m_size.height, ")");

        for (int i = 0; i < m_memo.size().total(); ++i)
        {
            m_memo.data()[i].iter = m_entropyQueue.push(EntropyQueueEntry{ m_memo.data()[i].entropy, i });
        }
    }

    Wave(const Wave&) = default;
    Wave(Wave&&) = default;
    Wave& operator=(const Wave&) = default;
    Wave& operator=(Wave&&) = default;
    ~Wave() = default;

    void reset()
    {
        m_hasContradiction = false;
        std::fill(std::begin(m_memo), std::end(m_memo), m_initEntry);
        m_numCompatibile = initNumCompatibile();
        m_canBePlaced.fill(true);
        auto dNoise = randomNoiseGenerator(m_noiseMax);
        for (auto& e : m_memo)
        {
            e.entropy += dNoise();
        }

        m_entropyQueue = {};
        for (int i = 0; i < m_memo.size().total(); ++i)
        {
            m_memo.data()[i].iter = m_entropyQueue.push(EntropyQueueEntry{ m_memo.data()[i].entropy, i });
        }
    }

    // should only be called after whole wave is defined
    // if everything went ok then it should always return a value
    [[nodiscard]] int probe(Coords2i pos) const
    {
        const int numElements = static_cast<int>(m_plogp.size());

        for (int i = 0; i < numElements; ++i)
        {
            if (m_canBePlaced[{pos, i}])
            {
                return i;
            }
        }

        // if we got here then it means that the wave is garbage
        // so why not return garbage
        return 0;
    }

    [[nodiscard]] Array2<int> probeAll() const
    {
        const Size2i waveSize = size();

        Array2<int> ids(waveSize);

        for (int x = 0; x < waveSize.width; ++x)
        {
            for (int y = 0; y < waveSize.height; ++y)
            {
                ids[x][y] = probe({ x, y });
            }
        }

        return ids;
    }

    [[nodiscard]] ObservationResult observeOnce(std::vector<float>& ps) noexcept
    {
        const auto [status, pos] = posWithMinimalEntropy();

        if (status == Wave::MinimalEntropyQueryResult::Contradiction)
        {
            return ObservationResult::Contradiction;
        }

        if (status == Wave::MinimalEntropyQueryResult::Finished)
        {
            return ObservationResult::Finished;
        }

        LOG_DEBUG(g_logger, "Observed (", pos.x, ", ", pos.y, ")");

        const int numPatterns = numElements();

        // choose an element according to the pattern distribution
        float pssum = 0.0f;
        {
            for (int i = 0; i < numPatterns; ++i)
            {
                pssum += canBePlaced(pos, i) ? m_p[i] : 0.0f;
                ps[i] = pssum;
            }
        }

        std::uniform_real_distribution<float> dPssum(0.0f, pssum);
        const float r = std::min(dPssum(m_rng), pssum); // min just in case of unfortunate rounding
        const auto iter = std::lower_bound(std::begin(ps), std::end(ps), r);
        const int patternId = static_cast<int>(std::distance(std::begin(ps), iter));

        setElement(pos, patternId);

        return ObservationResult::Unfinished;
    }

    [[nodiscard]] Size2i size() const
    {
        return m_size;
    }

    [[nodiscard]] int numElements() const
    {
        return static_cast<int>(m_plogp.size());
    }

    [[nodiscard]] bool canBePlaced(Coords2i pos, int elementId) const
    {
        return m_canBePlaced[{pos, elementId}];
    }

    void setElement(Coords2i pos, int elementId)
    {
        // define the cell with the chosen pattern by disabling others
        makeUnplacableAllExcept(pos, elementId);

        propagate();
    }

    [[nodiscard]] std::pair<MinimalEntropyQueryResult, Coords2i> posWithMinimalEntropy()
    {
        if (m_hasContradiction)
        {
            // there's cannot be an unassignable element
            return { MinimalEntropyQueryResult::Contradiction, {} };
        }

        // all settled
        if (m_entropyQueue.empty())
        {
            return { MinimalEntropyQueryResult::Finished, {} };
        }

        return { MinimalEntropyQueryResult::Success, m_memo.coordsFromFlatIndex(m_entropyQueue.top().index) };
    }

    void propagate()
    {
        switch (m_wrapping)
        {
        case WrappingMode::None:
            propagateImpl<WrappingMode::None>();
            break;
        case WrappingMode::Horizontal:
            propagateImpl<WrappingMode::Horizontal>();
            break;
        case WrappingMode::Vertical:
            propagateImpl<WrappingMode::Vertical>();
            break;
        case WrappingMode::All:
            propagateImpl<WrappingMode::All>();
            break;
        }
    }

private:
    void makeUnplacable(Coords2i pos, int elementId)
    {
        const int idx = m_canBePlaced.getFlatIndex({ pos, elementId });
        auto& canBePlaced = m_canBePlaced.data()[idx];
        if (!canBePlaced)
        {
            return;
        }

        canBePlaced = false;

        m_numCompatibile.data()[idx] = {};
        m_propagationQueue.emplace_back(pos, elementId);

        auto& memo = m_memo[pos];
        memo.plogpSum -= m_plogp[elementId];
        memo.pSum -= m_p[elementId];
        memo.numAvailableElements -= 1;
        if (memo.numAvailableElements == 0)
        {
            m_hasContradiction = true;
        }

        memo.entropy = util::approximateLog(memo.pSum) - memo.plogpSum / memo.pSum + randomNoiseGenerator(m_noiseMax)();
        if (memo.iter != EntropyQueueIterator{})
        {
            if (memo.numAvailableElements <= 1)
            {
                m_entropyQueue.erase(memo.iter);
                memo.iter = {};
            }
            else
            {
                memo.iter = m_entropyQueue.update(memo.iter, [entropy = memo.entropy](EntropyQueueEntry& e) {e.entropy = entropy; });
            }
        }
    }

    void makeUnplacableAllExcept(Coords2i pos, int preservedElementId)
    {
        const int end = numElements();

        for (int elementId = 0; elementId < end; ++elementId)
        {
            if (elementId == preservedElementId)
            {
                continue;
            }

            const int idx = m_canBePlaced.getFlatIndex({ pos, elementId });
            auto& canBePlaced = m_canBePlaced.data()[idx];
            if (canBePlaced)
            {
                m_numCompatibile.data()[idx] = {};
                m_propagationQueue.emplace_back(pos, elementId);
                canBePlaced = false;
            }
        }

        auto& memo = m_memo[pos];
        memo.plogpSum = m_plogp[preservedElementId];
        memo.pSum = m_p[preservedElementId];
        memo.numAvailableElements = m_canBePlaced[{pos, preservedElementId}];
        if (memo.numAvailableElements == 0)
        {
            m_hasContradiction = true;
        }
        // we don't need to change entropy since the values doesn't matter anymore anyway
        if (memo.iter != EntropyQueueIterator{})
        {
            m_entropyQueue.erase(memo.iter);
            memo.iter = {};
        }
    }

    template <WrappingMode WrapV>
    void propagateImpl()
    {
        while (!m_propagationQueue.empty())
        {
            const auto [x, y, elementId] = m_propagationQueue.back();
            m_propagationQueue.pop_back();

            applyOffsetAndPropagate<WrapV, Direction::North>(x, y, elementId);
            applyOffsetAndPropagate<WrapV, Direction::East>(x, y, elementId);
            applyOffsetAndPropagate<WrapV, Direction::South>(x, y, elementId);
            applyOffsetAndPropagate<WrapV, Direction::West>(x, y, elementId);
        }
    }

    // wraps to size of the wave
    template <WrappingMode WrapV, Direction DirV>
    void applyOffsetAndPropagate(int x, int y, int elementId)
    {
        constexpr int dx = offset(DirV).x;
        constexpr int dy = offset(DirV).y;
        constexpr bool hWrap = contains(WrapV, WrappingMode::Horizontal);
        constexpr bool vWrap = contains(WrapV, WrappingMode::Vertical);

        const Size2i waveSize = size();

        int x2 = x;
        int y2 = y;
        if constexpr (dx != 0)
        {
            x2 += dx;
            if constexpr (hWrap)
            {
                if constexpr (dx < 0)
                {
                    if (x2 < 0)
                    {
                        x2 = waveSize.width - 1;
                    }
                }
                else
                {
                    if (x2 >= waveSize.width)
                    {
                        x2 = 0;
                    }
                }
            }
            else if constexpr (dx < 0)
            {
                if (x2 < 0)
                {
                    return;
                }
            }
            else if (x2 >= waveSize.width)
            {
                return;
            }
        }

        if constexpr (dy != 0)
        {
            y2 += dy;
            if constexpr (vWrap)
            {
                if constexpr (dy < 0)
                {
                    if (y2 < 0)
                    {
                        y2 = waveSize.height - 1;
                    }
                }
                else
                {
                    if (y2 >= waveSize.height)
                    {
                        y2 = 0;
                    }
                }
            }
            else if constexpr (dy < 0)
            {
                if (y2 < 0)
                {
                    return;
                }
            }
            else if (y2 >= waveSize.height)
            {
                return;
            }
        }

        propagateTo(DirV, { x2, y2 }, elementId);
    }

    void propagateTo(Direction dir, Coords2i pos, int elementId)
    {
        const auto& compatibileElements = m_compatibile[elementId][dir];
        auto* numCompatibile = m_numCompatibile[pos];
        for (const int compatibileElementId : compatibileElements)
        {
            // decrease the number of compatibile elements
            // and handle the case when we end up with none compatibile left

            auto& count = numCompatibile[compatibileElementId][dir];
            count -= 1;

            if (count == 0)
            {
                makeUnplacable(pos, compatibileElementId);
            }
        }
    };
};
