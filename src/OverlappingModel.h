#pragma once

#include <array>
#include <map>
#include <vector>

#include "lib/pcg_random.hpp"

#include "Array2.h"
#include "NormalizedHistogram.h"
#include "Size2.h"
#include "SmallVector.h"
#include "D4Symmetry.h"
#include "WrappingMode.h"
#include "Logger.h"

struct OverlappingModelOptions
{
    static constexpr Size2i defaultOutputSize = { 32, 32 };
    static constexpr Size2i defaultStride = { 1, 1 };
    static constexpr int defaultPatternSize = 3;

    WrappingMode inputWrapping;
    WrappingMode outputWrapping;
    D4Symmetries symmetries;
    int patternSize; // pattern must be a square
    Size2i outputSize;

    // how far apart are wave grid points. {1,1} means that it's as dense as possible
    // increasing it speeds up observation process but may produce more artifacts
    Size2i stride;
    std::uint64_t seed;

    OverlappingModelOptions() :
        inputWrapping(WrappingMode::None),
        outputWrapping(WrappingMode::None),
        symmetries(D4Symmetries::None),
        patternSize(defaultPatternSize),
        outputSize(defaultOutputSize),
        stride(defaultStride),
        seed(123)
    {

    }

    [[nodiscard]] Size2i waveSize() const
    {
        const Size2i s = waveSizeUnstrided();
        return { s.width / stride.width, s.height / stride.height };
    }

    [[nodiscard]] bool isValid() const
    {
        const Size2i s = waveSizeUnstrided();
        return s.width % stride.width == 0 && s.height % stride.height == 0;
    }

    void setOutputSizeAtLeast(Size2i s)
    {
        const int dw =
            contains(outputWrapping, WrappingMode::Horizontal)
            ? 0
            : stride.width - patternSize;

        const int dh =
            contains(outputWrapping, WrappingMode::Vertical)
            ? 0
            : stride.height - patternSize;

        outputSize = { 
            ceilToMultiple(s.width, stride.width) - dw,
            ceilToMultiple(s.height, stride.height) - dh
        };
    }

private:
    [[nodiscard]] constexpr static int ceilToMultiple(int v, int m)
    {
        return (v - 1) / m * m + m;
    }

    [[nodiscard]] Size2i waveSizeUnstrided() const
    {
        const int width =
            contains(outputWrapping, WrappingMode::Horizontal)
            ? outputSize.width
            : outputSize.width - patternSize + stride.width;

        const int height =
            contains(outputWrapping, WrappingMode::Vertical)
            ? outputSize.height
            : outputSize.height - patternSize + stride.height;

        return Size2i(width, height);
    }
};

template <typename CellTypeT>
struct OverlappingModel
{
    using CellType = CellTypeT;
    using RandomNumberGeneratorType = pcg64_fast;

    OverlappingModel(Array2<CellType>&& input, const OverlappingModelOptions& options) :
        m_rng(options.seed),
        m_input(std::move(input)),
        m_options(options),
        m_patterns(gatherPatterns()),
        m_wave(computeCompatibilities(), options.waveSize(), m_patterns, m_options.outputWrapping)
    {
    }

    std::optional<Array2<CellType>> next()
    {
        m_wave.reset();
        for (;;)
        {
            switch (observeOne())
            {
            case ObservationResult::Contradiction:
                return std::nullopt;
            case ObservationResult::Finished:
                return decodeOutput();
            default:
                continue;
            }
        }
    }

    const OverlappingModelOptions& options() const
    {
        return m_options;
    }

private:
    RandomNumberGeneratorType m_rng;
    Array2<CellType> m_input;
    OverlappingModelOptions m_options;
    Patterns<CellType> m_patterns;
    Wave m_wave;

    enum struct ObservationResult
    {
        Contradiction,
        Finished,
        Unfinished
    };

    [[nodiscard]] Array2<CellType> decodeOutput() const
    {
        const Array2<int> wave = m_wave.probeAll();
        const Size2i waveSize = wave.size();

        const auto [sx, sy] = m_options.stride;

        Array2<CellType> out(m_options.outputSize);

        // fill places where we only need to read one value from the pattern
        for (int x = 0; x < waveSize.width; ++x)
        {
            for (int y = 0; y < waveSize.height; ++y)
            {
                const auto& pattern = m_patterns.element(wave[x][y]);
                for (int xx = 0; xx < sx; ++xx)
                {
                    for (int yy = 0; yy < sy; ++yy)
                    {
                        out[x*sx+xx][y*sy+yy] = pattern[xx][yy];
                    }
                }
            }
        }

        if (!contains(m_options.outputWrapping, WrappingMode::Horizontal))
        {
            // there are `m_options.patternSize - 1` columns left on the right side
            for (int dx = sx; dx < m_options.patternSize; ++dx)
            {
                for (int y = 0; y < waveSize.height; ++y)
                {
                    const auto& pattern = m_patterns.element(wave[waveSize.width - 1][y]);
                    for (int yy = 0; yy < sy; ++yy)
                    {
                        out[waveSize.width * sx + dx - sx][y * sy + yy] = pattern[dx][yy];
                    }
                }
            }
        }

        if (!contains(m_options.outputWrapping, WrappingMode::Vertical))
        {
            // there are `m_options.patternSize - 1` rows left on the bottom
            for (int x = 0; x < waveSize.width; ++x)
            {
                const auto& pattern = m_patterns.element(wave[x][waveSize.height - 1]);
                for (int dy = sy; dy < m_options.patternSize; ++dy)
                {
                    for (int xx = 0; xx < sx; ++xx)
                    {
                        out[x * sx + xx][waveSize.height * sy + dy - sy] = pattern[xx][dy];
                    }
                }
            }
        }

        if (m_options.outputWrapping == WrappingMode::None)
        {
            // fill the corner
            const auto& pattern = m_patterns.element(wave[waveSize.width - 1][waveSize.height - 1]);
            for (int dx = sx; dx < m_options.patternSize; ++dx)
            {
                for (int dy = sy; dy < m_options.patternSize; ++dy)
                {
                    out[waveSize.width * sx + dx - sx][waveSize.height * sy + dy - sy] = pattern[dx][dy];
                }
            }
        }

        return out;
    }

    [[nodiscard]] ObservationResult observeOne() noexcept
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

        LOG(g_logger, "Observed (", pos.x, ", ", pos.y, ")\n");

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

    // precomputed pattern adjacency compatibilities using overlapEqualWhenOffset
    [[nodiscard]] Wave::CompatibilityArrayType computeCompatibilities() const
    {
        LOG(g_logger, "Started computing compatibilities\n");

        const int numPatterns = m_patterns.size();

        Wave::CompatibilityArrayType compatibilities(numPatterns);

        for (int i = 0; i < numPatterns; ++i)
        {
            for (Direction dir : values<Direction>())
            {
                const Coords2i dirOffset = offset(dir);
                const Coords2i offset = {
                    dirOffset.x * m_options.stride.width,
                    dirOffset.y * m_options.stride.height
                };

                for (int j = 0; j < numPatterns; ++j)
                {
                    const auto& pattern1 = m_patterns.element(i);
                    const auto& pattern2 = m_patterns.element(j);

                    if (overlapEqualWhenOffset(pattern1, pattern2, offset))
                    {
                        compatibilities[i][dir].emplace_back(j);
                    }
                }
            }
        }

        LOG(g_logger, "Finished computing compatibilities\n");

        return compatibilities;
    }

    [[nodiscard]] Patterns<CellType> gatherPatterns() const
    {
        const Size2i inputSize = m_input.size();
        const int patternSize = m_options.patternSize;

        std::map<SquareArray2<CellType>, float> histogram;

        const int xbegin = 0;
        const int xend = 
            contains(m_options.inputWrapping, WrappingMode::Horizontal) 
            ? inputSize.width 
            : inputSize.width - patternSize + 1;
        const int ybegin = 0;
        const int yend =
            contains(m_options.inputWrapping, WrappingMode::Vertical)
            ? inputSize.height
            : inputSize.height - patternSize + 1;

        for (int x = xbegin; x < xend; ++x)
        {
            for (int y = ybegin; y < yend; ++y)
            {
                auto patterns = generateSymmetries(m_input.sub({ x, y }, patternSize, m_options.inputWrapping), m_options.symmetries);
                for (auto&& pattern : patterns)
                {
                    histogram[std::move(pattern)] += 1.0f;
                }
            }
        }

        LOG(g_logger, "Gathered ", histogram.size(), " patterns\n");

        return Patterns<CellType>(std::begin(histogram), std::end(histogram));
    }
};
