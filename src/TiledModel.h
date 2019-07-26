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

        const auto& adjacencies = tiles.adjacencies();

        for (const TileAdjacency& adj : adjacencies)
        {
            const auto& firstTile = tiles[adj.firstTileId];
            const auto& secondTile = tiles[adj.secondTileId];

            int i = 0;
            firstTile.forEachDistinct([&](const auto& pattern, D4Symmetry s1) {
                int j = 0;
                secondTile.forEachDistinct([&](const auto& pattern, D4Symmetry s2) {
                    const Direction firstDirection = mapping(s1)[adj.firstDirection];
                    const Direction secondDirection = mapping(s2)[adj.secondDirection];

                    if (!areOpposite(firstDirection, secondDirection))
                    {
                        return;
                    }

                    const int firstPatternId = flattenedIndex[adj.firstTileId] + i;
                    const int secondPatternId = flattenedIndex[adj.secondTileId] + j;

                    compatibilities[firstPatternId][firstDirection].emplace_back(secondPatternId);
                    compatibilities[secondPatternId][secondDirection].emplace_back(firstPatternId);

                    ++j;
                });

                i += 1;
            });
        }

        return compatibilities;
    }
};