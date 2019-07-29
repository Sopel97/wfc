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
#include "Model.h"

struct TiledModelOptions
{
    static constexpr Size2i defaultOutputSize = { 32, 32 };

    WrappingMode outputWrapping;
    Size2i outputSize;
    std::uint64_t seed;

    TiledModelOptions() :
        outputWrapping(WrappingMode::None),
        outputSize(defaultOutputSize),
        seed(123)
    {

    }

    [[nodiscard]] Size2i waveSize() const
    {
        return outputSize;
    }
};

template <typename CellTypeT>
struct TiledModel : Model<CellTypeT>
{
    using CellType = CellTypeT;
    using PatternsEntryType = std::pair<typename Patterns<CellType>::ElementType, float>;
    using TileSetType = TileSet<CellType>;
    using BaseType = Model<CellType>;

    TiledModel(const TileSetType& tiles, const TiledModelOptions& options) : 
        BaseType({options.seed}, flattenPatterns(tiles), computeCompatibilities(tiles), options.waveSize(), options.outputWrapping),
        m_options(options)
    {
    }

    const TiledModelOptions& options() const
    {
        return m_options;
    }

private:
    TiledModelOptions m_options;

    [[nodiscard]] Array2<CellType> decodeOutput() const override
    {
        const Array2<int> wave = this->wave().probeAll();
        const Size2i waveSize = wave.size();

        const int tileSize = this->patterns().element(0).size();

        Array2<CellType> out(m_options.outputSize * tileSize);

        for (int x = 0; x < waveSize.width; ++x)
        {
            for (int y = 0; y < waveSize.height; ++y)
            {
                const auto& pattern = this->patterns().element(wave[x][y]);

                for (int xx = 0; xx < tileSize; ++xx)
                {
                    for (int yy = 0; yy < tileSize; ++yy)
                    {
                        out[x * tileSize + xx][y * tileSize + yy] = pattern[xx][yy];
                    }
                }
            }
        }

        return out;
    }

    [[nodiscard]] static Patterns<CellType> flattenPatterns(const TileSetType& tiles)
    {
        std::vector<PatternsEntryType> patterns;

        for (const auto& tile : tiles.tiles())
        {
            tile.forEachDistinct([&patterns, &tile](const auto& pattern, D4Symmetry s) {
                patterns.emplace_back(pattern, tile.weight());
                });
        }

        return Patterns<CellType>(std::begin(patterns), std::end(patterns));
    }

    // ensures index compatibility with output from flattenPatterns()
    [[nodiscard]] static Wave::CompatibilityArrayType computeCompatibilities(const TileSetType& tiles)
    {
        std::vector<int> flattenedIndex;
        flattenedIndex.reserve(tiles.size());

        int numPatterns = 0;
        for (const auto& tile : tiles.tiles())
        {
            flattenedIndex.emplace_back(numPatterns);
            numPatterns += tile.numDistinct();
        }

        Wave::CompatibilityArrayType compatibilities(numPatterns);

        const int numTiles = tiles.size();
        for (int firstTileId = 0; firstTileId < numTiles; ++firstTileId)
        {
            const auto& firstTile = tiles[firstTileId];

            for (int secondTileId = firstTileId; secondTileId < numTiles; ++secondTileId)
            {
                const auto& secondTile = tiles[secondTileId];

                int i = 0;
                firstTile.forEachDistinct([&](const auto& pattern, D4Symmetry s1) {
                    int j = 0;
                    secondTile.forEachDistinct([&](const auto& pattern, D4Symmetry s2) {
                        // we try to put the two tiles in all possible side by side configurations
                        for (Direction connectionDir : values<Direction>())
                        {
                            // we need to know to which original side each transformed side
                            // corresponds to
                            const Direction firstOriginalDirection = invMapping(s1)[connectionDir];
                            const Direction secondOriginalDirection = invMapping(s2)[oppositeTo(connectionDir)];

                            if (firstTile.sideIds()[firstOriginalDirection] == secondTile.sideIds()[secondOriginalDirection])
                            {
                                const int firstPatternId = flattenedIndex[firstTileId] + i;
                                const int secondPatternId = flattenedIndex[secondTileId] + j;

                                compatibilities[firstPatternId][connectionDir].emplace_back(secondPatternId);
                                compatibilities[secondPatternId][oppositeTo(connectionDir)].emplace_back(firstPatternId);
                            }
                        }

                        ++j;
                    });

                    ++i;
                });
            }
        }

        return compatibilities;
    }
};