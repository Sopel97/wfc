#pragma once

#include <vector>
#include <utility>

#include "Array2.h"
#include "D4Symmetry.h"
#include "SmallVector.h"
#include "NormalizedHistogram.h"

// TODO: maybe some more rules to disallow all connections between two tiles.
//       needed for example for circuit's vias and viad
struct TileConnectivity
{
    ByDirection<int> sideId;
    ByDirection<int> mirroredSideId;
    bool allowsSelfConnections;

    TileConnectivity(ByDirection<int> sideId) :
        sideId(sideId),
        mirroredSideId(sideId),
        allowsSelfConnections(true)
    {

    }

    TileConnectivity(ByDirection<int> sideId, bool b) :
        sideId(sideId),
        mirroredSideId(sideId),
        allowsSelfConnections(b)
    {

    }

    TileConnectivity(ByDirection<int> sideId, ByDirection<int> mirroredSideId) :
        sideId(sideId),
        mirroredSideId(mirroredSideId),
        allowsSelfConnections(true)
    {

    }

    TileConnectivity(ByDirection<int> sideId, ByDirection<int> mirroredSideId, bool b) :
        sideId(sideId),
        mirroredSideId(mirroredSideId),
        allowsSelfConnections(b)
    {

    }
};

template <typename CellTypeT>
struct Tile
{
    using PatternType = SquareArray2<CellTypeT>;
    using SideIdType = std::pair<int, int>;

    Tile(PatternType&& basePattern, const TileConnectivity& connectivity, D4SymmetriesClosure symmetries, float weight) :
        m_distinctPatterns{},
        m_connectivity(connectivity),
        m_symmetries(symmetries),
        m_missingSymmetries(missing(symmetries)),
        m_weight(weight)
    {
        m_distinctPatterns = generateSymmetries(std::move(basePattern), m_missingSymmetries);
    }

    Tile(const Tile&) = default;
    Tile(Tile&&) = default;
    Tile& operator=(const Tile&) = default;
    Tile& operator=(Tile&&) = default;

    // FuncT(TilePattern, D4Symmetry)
    template <typename FuncT>
    void forEachDistinct(FuncT&& f) const
    {
        f(m_distinctPatterns[0], D4Symmetry::Rotation0);
        int i = 0;
        forEach(m_missingSymmetries, [this, &f, &i](D4Symmetry s) {
            ++i;
            f(m_distinctPatterns[i], s);
        });
    }

    // FuncT(TilePattern, D4Symmetry)
    template <typename FuncT>
    void forEachDistinct(FuncT&& f)
    {
        f(m_distinctPatterns[0], D4Symmetry::Rotation0);
        int i = 0;
        forEach(m_missingSymmetries, [this, &f, &i](D4Symmetry s) {
            ++i;
            f(m_distinctPatterns[i], s);
        });
    }

    int numDistinct() const
    {
        return m_distinctPatterns.size();
    }

    float weight() const
    {
        return m_weight;
    }

    PatternType& operator[](D4Symmetry s)
    {
        return m_distinctPatterns[indexOf(s)];
    }

    const PatternType& operator[](D4Symmetry s) const
    {
        return m_distinctPatterns[indexOf(s)];
    }

    const TileConnectivity& connectivity() const
    {
        return m_connectivity;
    }

    const int sideId(Direction side, D4Symmetry transform, bool mirror) const
    {
        // we need to know to which original side each transformed side
        // corresponds to
        const Direction originalSide = invMapping(transform)[side];
        const int isMirror = isMirroring(transform) ^ mirror;
        return isMirror ? m_connectivity.mirroredSideId[originalSide] : m_connectivity.sideId[originalSide]; 
    }

    bool allowsSelfConnections() const
    {
        return m_connectivity.allowsSelfConnections;
    }

private:
    SmallVector<PatternType, 8> m_distinctPatterns;
    TileConnectivity m_connectivity;
    D4SymmetriesClosure m_symmetries;
    D4Symmetries m_missingSymmetries; // symmetries that produce the m_distinctPatterns
    float m_weight;

    int indexOf(D4Symmetry symmetry) const
    {
        if (symmetry == D4Symmetry::Rotation0)
        {
            return 0;
        }
        else if (contains(m_missingSymmetries, symmetry))
        {
            int i = 0;
            for (D4Symmetry s : m_missingSymmetries)
            {
                ++i;
                if (s == symmetry)
                {
                    return i;
                }
            }
        }
        else
        {
            int i = 0;
            for (D4Symmetry s : m_missingSymmetries)
            {
                ++i;
                if (areEquivalentUnderSymmetries(m_symmetries, s, symmetry))
                {
                    return i;
                }
            }
        }

        // shouldn't happen?
        return 0;
    }
};

template <typename CellTypeT>
struct TileSet
{
    using CellType = CellTypeT;
    using TileType = Tile<CellType>;
    using TileArrayType = std::vector<TileType>;

    TileSet() = default;

    int emplace(TileType&& tile)
    {
        m_tiles.emplace_back(std::move(tile));
        return size() - 1;
    }

    TileType& operator[](int i)
    {
        return m_tiles[i];
    }

    const TileType& operator[](int i) const
    {
        return m_tiles[i];
    }

    int size() const
    {
        return static_cast<int>(m_tiles.size());
    }

    const TileArrayType& tiles() const &
    {
        return m_tiles;
    }

    TileArrayType&& tiles() &&
    {
        return std::move(m_tiles);
    }

private:
    TileArrayType m_tiles;
};
