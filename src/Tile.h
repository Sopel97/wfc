#pragma once

#include "Array2.h"
#include "D4Symmetry.h"
#include "SmallVector.h"

template <typename CellTypeT>
struct Tile
{
    using TilePattern = SquareArray2<CellTypeT>;

    Tile(TilePattern&& basePattern, D4Symmetries symmetries, float weigth) :
        m_distinctPatterns{},
        m_symmetries(symmetries),
        m_missingSymmetries(D4SymmetryHelper::missing(symmetries)),
        m_weigth(weigth)
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
        for (D4Symmetry s : m_missingSymmetries)
        {
            ++i;
            f(m_distinctPatterns[i], s);
        }
    }

    int numDistinct() const
    {
        return m_distinctPatterns.size();
    }

    float weigth() const
    {
        return m_weigth;
    }

private:
    SmallVector<TilePattern, 8> m_distinctPatterns;
    D4Symmetries m_symmetries;
    D4Symmetries m_missingSymmetries; // symmetries that produce the m_distinctPatterns
    float m_weigth;
};
