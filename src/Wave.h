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

    Entry m_initEntry;

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
        const int numElements = static_cast<int>(m_plogp.size());

        Array3<ByDirection<int>> res({ width, height, numElements });

        for (int x = 0; x < width; ++x)
        {
            for (int y = 0; y < height; ++y)
            {
                for (int elementId = 0; elementId < numElements; ++elementId)
                {
                    const auto& compatibile = m_compatibile[elementId];
                    
                    ByDirection<int> counts{};

                    for (Direction dir : values<Direction>())
                    {
                        counts[dir] = static_cast<int>(compatibile[oppositeTo(dir)].size());
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

        m_initEntry = Entry{ baseEntropy, 1.0f, static_cast<int>(freq.size()), -baseEntropy };

        std::fill(std::begin(m_memo), std::end(m_memo), m_initEntry);

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

    void reset()
    {
        std::fill(std::begin(m_memo), std::end(m_memo), m_initEntry);
        m_numCompatibile = initNumCompatibile();
        m_canBePlaced.fill(true);
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

    [[nodiscard]] Size2i size() const
    {
        return m_memo.size();
    }

    [[nodiscard]] bool canBePlaced(Coords2i pos, int elementId) const
    {
        return m_canBePlaced[{pos, elementId}];
    }

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
        memo.pSum -= (*m_p)[elementId];
        memo.numAvailableElements -= 1;
        memo.entropy = std::log(memo.pSum) - memo.plogpSum / memo.pSum;
    }

    template <typename RngT>
    __declspec(noinline) [[nodiscard]] std::pair<MinimalEntropyQueryResult, Coords2i> posWithMinimalEntropy(RngT&& rng) const
    {
        constexpr int invalidArg = -1;

        std::uniform_real_distribution<float> dNoise(0.0f, m_noiseMax);

        float minEntropy = std::numeric_limits<float>::max();
        int minArg = invalidArg;

        auto [width, height] = size();

        for (int i = 0; i < width * height; ++i)
        {
            const auto& memo = m_memo.data()[i];
            const int numAvailable = memo.numAvailableElements;
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
            const float entropy = memo.entropy;
            if (entropy < minEntropy)
            {
                // TODO: if we can move this rng call somewhere else then this
                // loop can be parallelised and preserve determinism
                // maybe update noise in `makeUnplacable`?
                const float noise = dNoise(rng);
                if (entropy + noise < minEntropy)
                {
                    minEntropy = entropy + noise;
                    minArg = i;
                }
            }
        }

        // all settled
        if (minArg == invalidArg)
        {
            return { MinimalEntropyQueryResult::Finished, {} };
        }

        return { MinimalEntropyQueryResult::Success, m_memo.coordsFromFlatIndex(minArg) };
    }

    void propagate()
    {
        switch (m_wrapping)
        {
        case WrappingMode::None:
            propagateImpl<WrappingMode::None>();
        case WrappingMode::Horizontal:
            propagateImpl<WrappingMode::Horizontal>();
        case WrappingMode::Vertical:
            propagateImpl<WrappingMode::Vertical>();
        case WrappingMode::All:
            propagateImpl<WrappingMode::All>();
        }
    }

private:
    template <WrappingMode WrapV>
    __declspec(noinline) void propagateImpl()
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
                x2 = (x2 + waveSize.width) % waveSize.width;
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
                y2 = (y2 + waveSize.height) % waveSize.height;
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
