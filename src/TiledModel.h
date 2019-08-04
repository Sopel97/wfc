#pragma once

#include <array>
#include <map>
#include <utility>
#include <vector>

#include "lib/pcg_random.hpp"

#include "Array2.h"
#include "D4Symmetry.h"
#include "Logger.h"
#include "Model.h"
#include "NormalizedHistogram.h"
#include "Size2.h"
#include "SmallVector.h"
#include "WrappingMode.h"

template <typename CellTypeT>
struct TiledModelOptions
{
    using SeedType = typename Model<CellTypeT>::ModelSeedType;

    static constexpr Size2i defaultOutputSize = { 32, 32 };

    WrappingMode outputWrapping;
    Size2i outputSize;
    SeedType seed;

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
    using TileType = typename TileSetType::TileType;
    using BaseType = Model<CellType>;
    using CompatibilityArrayType = typename BaseType::CompatibilityArrayType;
    using OptionsType = TiledModelOptions<CellType>;

    TiledModel(const TileSetType& tiles, const OptionsType& options) :
        BaseType(flattenPatterns(tiles), computeCompatibilities(tiles), options.seed),
        m_options(options)
    {
        LOG_INFO(g_logger, "Created tiled model");
    }

    [[nodiscard]] const OptionsType& options() const
    {
        return m_options;
    }

private:
    OptionsType m_options;

    [[nodiscard]] Array2<CellType> decodeOutput(Wave&& wave) const override
    {
        const Array2<int> waveValues = wave.probeAll();
        const Size2i waveSize = waveValues.size();

        const int tileSize = this->patterns().element(0).size();

        Array2<CellType> out(m_options.outputSize * tileSize);

        for (int x = 0; x < waveSize.width; ++x)
        {
            for (int y = 0; y < waveSize.height; ++y)
            {
                const auto& pattern = this->patterns().element(waveValues[x][y]);

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

    [[nodiscard]] Size2i waveSize() const override
    {
        return m_options.waveSize();
    }

    [[nodiscard]] WrappingMode outputWrapping() const override
    {
        return m_options.outputWrapping;
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

        LOG_INFO(g_logger, "Gathered ", patterns.size(), " patterns");

        return Patterns<CellType>(std::begin(patterns), std::end(patterns));
    }

    // ensures index compatibility with output from flattenPatterns()
    [[nodiscard]] static CompatibilityArrayType computeCompatibilities(const TileSetType& tiles)
    {
        std::vector<int> flattenedIndex;
        flattenedIndex.reserve(tiles.size());

        int numPatterns = 0;
        for (const auto& tile : tiles.tiles())
        {
            flattenedIndex.emplace_back(numPatterns);
            numPatterns += tile.numDistinct();
        }

        CompatibilityArrayType compatibilities(numPatterns);

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
                            if (areSidesCompatibile(tiles, firstTile, s1, secondTile, s2, connectionDir))
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

    [[nodiscard]] static bool areSidesCompatibile(const TileSetType& tiles, const TileType& firstTile, D4Symmetry firstTransform, const TileType& secondTile, D4Symmetry secondTransform, Direction connectionDir)
    {
        const int firstSideId = firstTile.sideId(connectionDir, firstTransform, false);
        const int secondSideId = secondTile.sideId(oppositeTo(connectionDir), secondTransform, true);

        return firstSideId == secondSideId && tiles.areCompatibile(firstTile.id(), secondTile.id(), firstSideId);
    }
};