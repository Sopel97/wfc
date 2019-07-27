#pragma once

#include <utility>
#include <vector>

#include "lib/pcg_random.hpp"

#include "Tile.h"
#include "Wave.h"

template <typename CellTypeT>
struct TiledModel
{
    using CellType = CellTypeT;
    using RandomNumberGeneratorType = pcg64_fast;
    using PatternsEntryType = std::pair<typename Patterns<CellType>::ElementType, float>;
    using TileSetType = TileSet<CellType>;

    TiledModel(TileSetType&& tiles)
    {
        computeCompatibilities(tiles);
        m_patterns = flattenPatterns(std::move(tiles));
    }

private:
    RandomNumberGeneratorType m_rng;
    Patterns<CellType> m_patterns;

    [[nodiscard]] Patterns<CellType> flattenPatterns(TileSetType&& tiles) const
    {
        std::vector<PatternsEntryType> patterns;

        for (auto&& tile : std::move(tiles).tiles())
        {
            std::move(tile).forEachDistinct([&patterns, &tile](auto&& pattern, D4Symmetry s) {
                patterns.emplace_back(std::move(pattern), tile.weight());
                });
        }

        return Patterns<CellType>(std::begin(patterns), std::end(patterns));
    }

    // ensures index compatibility with output from flattenPatterns()
    [[nodiscard]] Wave::CompatibilityArrayType computeCompatibilities(const TileSetType& tiles) const
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

        int firstTileId = 0;
        for (const auto& firstTile : tiles.tiles())
        {
            int secondTileId = 0;
            for (const auto& secondTile : tiles.tiles())
            {
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

                                // TODO: maybe go through a dense bool array first to ensure there are no duplicates?
                                compatibilities[firstPatternId][connectionDir].emplace_back(secondPatternId);
                            }
                        }

                        ++j;
                    });

                    ++i;
                });
                ++secondTileId;
            }
            ++firstTileId;
        }

        return compatibilities;
    }
};