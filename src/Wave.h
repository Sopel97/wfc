#pragma once

#include <algorithm>
#include <cmath>
#include <iterator>
#include <optional>
#include <queue>
#include <random>

#include "Array2.h"
#include "Array3.h"
#include "Direction.h"
#include "NormalizedHistogram.h"
#include "Logger.h"

struct Wave
{
    using CompatibilityArrayType = std::vector<ByDirection<std::vector<int>>>;

private:
    struct Entry
    {
        // sum { p'(element) * log(p'(element)) }
        float plogpSum;

        // sum { p'(element) }
        float pSum;

        int numAvailableElements;

        float entropy;
    };

    WrappingMode m_wrapping;

    // p
    const std::vector<float>* m_p;

    // p * log(p)
    std::vector<float> m_plogp;

    Array2<Entry> m_memo;

    // min { (p * log(p)) / 2 }
    float m_noiseMax;

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

    Array3<ByDirection<int>> initNumCompatibile() const
    {
        const auto [width, height] = size();
        const int numElements = m_plogp.size();

        Array3<ByDirection<int>> res({ width, height, numElements });

        for (int x = 0; x < width; ++x)
        {
            for (int y = 0; y < height; ++y)
            {
                for (int elementId = 0; elementId < numElements; ++elementId)
                {
                    const auto& compatibile = m_compatibile[elementId];
                    
                    ByDirection<int> counts{};

                    for (Direction dir : DirectionHelper::values())
                    {
                        counts[dir] = compatibile[DirectionHelper::oppositeTo(dir)].size();
                    }

                    res[{x, y, elementId}] = counts;
                }
            }
        }

        return res;
    }

public:
    enum struct MinimalEntropyQueryResult
    {
        Success,
        Contradiction,
        Finished
    };

    Wave(CompatibilityArrayType&& compatibility, Size2i size, const NormalizedFrequencies& freq, WrappingMode wrapping) :
        m_wrapping(wrapping),
        m_p(&freq.frequencies()),
        m_plogp(freq.size()),
        m_memo(size),
        m_noiseMax(std::numeric_limits<float>::max()),
        m_canBePlaced(Size3i(size, freq.size()), true),
        m_compatibile(std::move(compatibility)),
        m_numCompatibile(initNumCompatibile())
    {
        std::transform(
            std::begin(*m_p), 
            std::end(*m_p),
            std::begin(m_plogp), 
            [](float p) { return p * std::log(p); }
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

        std::fill(std::begin(m_memo), std::end(m_memo), Entry{ baseEntropy, 1.0f, static_cast<int>(freq.size()), -baseEntropy });

        LOG(g_logger, "baseEntropy = ", baseEntropy, "\n");
        LOG(g_logger, "numAvailableElements = ", freq.size(), "\n");
        LOG(g_logger, "entropy = ", -baseEntropy, "\n");
        LOG(g_logger, "noiseMax = ", m_noiseMax, "\n");
    }

    Wave(const Wave&) = default;
    Wave(Wave&&) = default;
    Wave& operator=(const Wave&) = default;
    Wave& operator=(Wave&&) = default;
    ~Wave() = default;

    // should only be called after whole wave is defined
    // if everything went ok then it should always return a value
    [[nodiscard]] int probe(Coords2i pos) const
    {
        const int numElements = m_plogp.size();

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

    [[nodiscard]] Size2i size() const
    {
        return m_memo.size();
    }

    [[nodiscard]] bool canBePlaced(Coords2i pos, int elementId) const
    {
        return m_canBePlaced[{pos, elementId}];
    }

    // returns number of patterns left
    int makeUnplacable(Coords2i pos, int elementId)
    {
        if (!canBePlaced(pos, elementId))
        {
            return m_memo[pos].numAvailableElements;
        }

        schedulePropagationOnElementMadeUnavailable(pos, elementId);

        m_canBePlaced[{pos, elementId}] = false;

        auto& memo = m_memo[pos];
        memo.plogpSum -= m_plogp[elementId];
        memo.pSum -= (*m_p)[elementId];
        memo.numAvailableElements -= 1;
        memo.entropy = std::log(memo.pSum) - memo.plogpSum / memo.pSum;

        return memo.numAvailableElements;
    }

    template <typename RngT>
    [[nodiscard]] std::pair<MinimalEntropyQueryResult, Coords2i> posWithMinimalEntropy(RngT&& rng) const
    {
        static constexpr Coords2i invalidCoords(-1, -1);

        std::uniform_real_distribution<float> dNoise(0.0f, m_noiseMax);

        float minEntropy = std::numeric_limits<float>::max();
        Coords2i minArg = invalidCoords;

        auto [width, height] = size();

        for (int x = 0; x < width; ++x)
        {
            for (int y = 0; y < height; ++y)
            {
                const Coords2i pos(x, y);

                const int numAvailable = m_memo[pos].numAvailableElements;
                if (numAvailable == 0)
                {
                    // there's cannot be an unassignable element
                    return { MinimalEntropyQueryResult::Contradiction, {} };
                }
                else if (numAvailable == 1)
                {
                    // already settled
                    continue;
                }

                // still in superposition
                const float entropy = m_memo[pos].entropy;
                if (entropy < minEntropy)
                {
                    const float noise = dNoise(rng);
                    if (entropy + noise < minEntropy)
                    {
                        minEntropy = entropy + noise;
                        minArg = pos;
                    }
                }
            }
        }

        // all settled
        if (minArg == invalidCoords)
        {
            return { MinimalEntropyQueryResult::Finished, {} };
        }

        return { MinimalEntropyQueryResult::Success, minArg };
    }

    void schedulePropagationOnElementMadeUnavailable(Coords2i pos, int elementId)
    {
        const Coords3i c(pos, elementId);
        m_numCompatibile[c] = {};
        m_propagationQueue.emplace_back(c);
    }

    void propagate()
    {
        const Size2i waveSize = size();

        while (!m_propagationQueue.empty())
        {
            const auto [x, y, elementId] = m_propagationQueue.back();
            m_propagationQueue.pop_back();

            for (const Direction dir : DirectionHelper::values())
            {
                const auto [dx, dy] = DirectionHelper::offset(dir);

                int x2 = x + dx;
                if (contains(m_wrapping, WrappingMode::Horizontal))
                {
                    x2 = (x2 + waveSize.width) % waveSize.width;
                }
                else if (x2 < 0 || x2 >= waveSize.width)
                {
                    continue;
                }

                int y2 = y + dy;
                if (contains(m_wrapping, WrappingMode::Vertical))
                {
                    y2 = (y2 + waveSize.height) % waveSize.height;
                }
                else if (y2 < 0 || y2 >= waveSize.height)
                {
                    continue;
                }

                const auto& compatibileElements = m_compatibile[elementId][dir];
                for (const int compatibileElementId : compatibileElements)
                {
                    // decrease the number of compatibile elements
                    // and handle the case when we end up with none compatibile left

                    auto& counts = m_numCompatibile[{ x2, y2, compatibileElementId }];
                    counts[dir] -= 1;

                    if (counts[dir] == 0) 
                    {
                        makeUnplacable({ x2, y2 }, compatibileElementId);
                    }
                }
            }
        }
    }
};
