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
#include "Model.h"

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
struct OverlappingModel : Model<CellTypeT>
{
    using CellType = CellTypeT;
    using BaseType = Model<CellType>;

    OverlappingModel(const Array2<CellType>& input, const OverlappingModelOptions& options) :
        // let's hope the compiler will call gatherPatterns only once
        BaseType(gatherPatterns(input, options), computeCompatibilities(input, options), options.seed, options.waveSize(), options.outputWrapping),
        m_options(options)
    {
        LOG_INFO(g_logger, "Created overlapping model");
    }

    const OverlappingModelOptions& options() const
    {
        return m_options;
    }

private:
    OverlappingModelOptions m_options;

    [[nodiscard]] Array2<CellType> decodeOutput() const override
    {
        const Array2<int> wave = this->wave().probeAll();
        const Size2i waveSize = wave.size();

        auto [sx, sy] = m_options.stride;

        Array2<CellType> out(m_options.outputSize);

        // fill places where we only need to read one value from the pattern
        for (int x = 0; x < waveSize.width; ++x)
        {
            for (int y = 0; y < waveSize.height; ++y)
            {
                const auto& pattern = this->patterns().element(wave[x][y]);
                for (int xx = 0; xx < sx; ++xx)
                {
                    for (int yy = 0; yy < sy; ++yy)
                    {
                        out[x * sx + xx][y * sy + yy] = pattern[xx][yy];
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
                    const auto& pattern = this->patterns().element(wave[waveSize.width - 1][y]);
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
                const auto& pattern = this->patterns().element(wave[x][waveSize.height - 1]);
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
            const auto& pattern = this->patterns().element(wave[waveSize.width - 1][waveSize.height - 1]);
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

    // precomputed pattern adjacency compatibilities using overlapEqualWhenOffset
    [[nodiscard]] static Wave::CompatibilityArrayType computeCompatibilities(const Array2<CellType>& input, const OverlappingModelOptions& options)
    {
        const auto patterns = gatherPatterns(input, options);

        LOG_INFO(g_logger, "Gathered ", patterns.size(), " patterns");

        const int numPatterns = patterns.size();

        Wave::CompatibilityArrayType compatibilities(numPatterns);

        for (int i = 0; i < numPatterns; ++i)
        {
            for (Direction dir : values<Direction>())
            {
                const Coords2i dirOffset = offset(dir);
                const Coords2i offset = {
                    dirOffset.x * options.stride.width,
                    dirOffset.y * options.stride.height
                };

                for (int j = 0; j < numPatterns; ++j)
                {
                    const auto& pattern1 = patterns.element(i);
                    const auto& pattern2 = patterns.element(j);

                    if (overlapEqualWhenOffset(pattern1, pattern2, offset))
                    {
                        compatibilities[i][dir].emplace_back(j);
                    }
                }
            }
        }

        return compatibilities;
    }

    [[nodiscard]] static Patterns<CellType> gatherPatterns(const Array2<CellType>& input, const OverlappingModelOptions& options)
    {
        const Size2i inputSize = input.size();
        const int patternSize = options.patternSize;

        std::map<SquareArray2<CellType>, float> histogram;

        const int xbegin = 0;
        const int xend = 
            contains(options.inputWrapping, WrappingMode::Horizontal)
            ? inputSize.width 
            : inputSize.width - patternSize + 1;
        const int ybegin = 0;
        const int yend =
            contains(options.inputWrapping, WrappingMode::Vertical)
            ? inputSize.height
            : inputSize.height - patternSize + 1;

        for (int x = xbegin; x < xend; ++x)
        {
            for (int y = ybegin; y < yend; ++y)
            {
                auto patterns = generateSymmetries(input.sub({ x, y }, patternSize, options.inputWrapping), options.symmetries);
                for (auto&& pattern : patterns)
                {
                    histogram[std::move(pattern)] += 1.0f;
                }
            }
        }

        return Patterns<CellType>(std::begin(histogram), std::end(histogram));
    }
};
