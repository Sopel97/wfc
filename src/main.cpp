#include <iostream>
#include <filesystem>
#include <map>
#include <set>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

#include "Array2.h"
#include "Wave.h"
#include "Array3.h"
#include "Color.h"
#include "Direction.h"
#include "OverlappingModel.h"
#include "NormalizedHistogram.h"
#include "SmallVector.h"
#include "Wave.h"
#include "WrappingMode.h"
#include "Tile.h"
#include "D4Symmetry.h"
#include "TiledModel.h"
#include "Logger.h"
#include "UpdatablePriorityQueue.h"

static inline Array2<ColorRGBi> loadImage(const std::string& path)
{
    LOG_INFO(g_logger, "Loading ", path);

    int width, height, channels;
    unsigned char* data = stbi_load(
        path.c_str(),
        &width,
        &height,
        &channels,
        STBI_rgb
    );

    const unsigned char* current = &(data[0]);

    Array2<ColorRGBi> image({ width, height });

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            image[x][y] = ColorRGBi(
                current[0],
                current[1],
                current[2]
            );

            current += 3;
        }
    }

    stbi_image_free(data);

    return image;
}

static inline void saveImage(const Array2<ColorRGBi>& image, const std::string& path)
{
    LOG_INFO(g_logger, "Saving ", path);

    std::vector<unsigned char> data;
    data.reserve(image.size().total() * 3);

    auto [width, height] = image.size();

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            auto c = image[x][y];

            data.emplace_back(c.r);
            data.emplace_back(c.g);
            data.emplace_back(c.b);
        }
    }

    stbi_write_png(path.c_str(), width, height, STBI_rgb, data.data(), width * STBI_rgb);
}

enum struct KnotTileSetSubset
{
    All,
    Standard,
    Dense,
    Crossless,
    TE,
    T,
    CL,
    CE,
    C,
    Fabric,
    DenseFabric
};

TileSet<ColorRGBi> makeKnotTileSet(KnotTileSetSubset subset = KnotTileSetSubset::All)
{
    TileSet<ColorRGBi> ts;

    // side ids
    constexpr int e = 0;
    constexpr int p = 1;

    const int corner = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/knot/corner.png").square(), { ByDirection<int>::nesw(p, p, e, e) }, D4SymmetryHelper::closureFromChar('L'), 1.0f));
    const int cross = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/knot/cross.png").square(), { ByDirection<int>::nesw(p, p, p, p) }, D4SymmetryHelper::closureFromChar('I'), 1.0f));
    const int empty = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/knot/empty.png").square(), { ByDirection<int>::nesw(e, e, e, e) }, D4SymmetryHelper::closureFromChar('X'), 1.0f));
    const int line = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/knot/line.png").square(), { ByDirection<int>::nesw(e, p, e, p) }, D4SymmetryHelper::closureFromChar('I'), 1.0f));
    const int t = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/knot/t.png").square(), {ByDirection<int>::nesw(e, p, p, p) }, D4SymmetryHelper::closureFromChar('T'), 1.0f));

    if (subset != KnotTileSetSubset::All)
    {
        std::map<KnotTileSetSubset, std::set<int>> subsets = {
            { KnotTileSetSubset::Standard, { corner, cross, empty, line } },
            { KnotTileSetSubset::Dense, { corner, cross, line } },
            { KnotTileSetSubset::Crossless, { corner, empty, line } },
            { KnotTileSetSubset::TE, { empty, t } },
            { KnotTileSetSubset::T, { t } },
            { KnotTileSetSubset::CL, { corner, line } },
            { KnotTileSetSubset::CE, { corner, empty } },
            { KnotTileSetSubset::C, { corner } },
            { KnotTileSetSubset::Fabric, { cross, line } },
            { KnotTileSetSubset::DenseFabric, { cross } }
        };

        return ts.subset(subsets[subset]).first;
    }
    else
    {
        return ts;
    }
}

TileSet<ColorRGBi> makeCircuitTileSet()
{
    TileSet<ColorRGBi> ts;

    enum {
        sub,
        wir,
        tra,
        co0,
        co1,
        com
    };

    const int wire = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/circuit/wire.png").square(), { ByDirection<int>::nesw(sub, wir, sub, wir) }, D4SymmetryHelper::closureFromChar('I'), 0.5f));
    const int vias = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/circuit/vias.png").square(), { ByDirection<int>::nesw(tra, sub, sub, sub), }, D4SymmetryHelper::closureFromChar('T'), 0.3f));
    const int viad = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/circuit/viad.png").square(), { ByDirection<int>::nesw(sub, tra, sub, tra), }, D4SymmetryHelper::closureFromChar('I'), 0.1f));
    const int transition = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/circuit/transition.png").square(), { ByDirection<int>::nesw(wir, sub, tra, sub) }, D4SymmetryHelper::closureFromChar('T'), 0.4f));
    const int track = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/circuit/track.png").square(), { ByDirection<int>::nesw(tra, sub, tra, sub) }, D4SymmetryHelper::closureFromChar('I'), 2.0f));
    const int t = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/circuit/t.png").square(), { ByDirection<int>::nesw(sub, tra, tra, tra) }, D4SymmetryHelper::closureFromChar('T'), 0.1f));
    const int substrate = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/circuit/substrate.png").square(), { ByDirection<int>::nesw(sub, sub, sub, sub) }, D4SymmetryHelper::closureFromChar('X'), 2.0f));
    const int skew = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/circuit/skew.png").square(), { ByDirection<int>::nesw(tra, tra, sub, sub) }, D4SymmetryHelper::closureFromChar('L'), 2.0f));
    const int dskew = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/circuit/dskew.png").square(), { ByDirection<int>::nesw(tra, tra, tra, tra) }, D4SymmetryHelper::closureFromChar('%'), 2.0f));
    const int corner = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/circuit/corner.png").square(), { ByDirection<int>::nesw(sub, sub, co0, co1), ByDirection<int>::nesw(sub, sub, co1, co0) }, D4SymmetryHelper::closureFromChar('L'), 10.0f));
    const int connection = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/circuit/connection.png").square(), { ByDirection<int>::nesw(tra, co0, com, co1), ByDirection<int>::nesw(tra, co1, com, co0) }, D4SymmetryHelper::closureFromChar('T'), 10.0f));
    const int component = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/circuit/component.png").square(), { ByDirection<int>::nesw(com, com, com, com) }, D4SymmetryHelper::closureFromChar('X'), 20.0f));
    const int bridge = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/circuit/bridge.png").square(), { ByDirection<int>::nesw(tra, wir, tra, wir) }, D4SymmetryHelper::closureFromChar('I'), 1.0f));

    ts.makeIncompatibile(vias, vias, tra);
    ts.makeIncompatibile(viad, viad, tra);
    ts.makeIncompatibile(vias, viad, tra);
    ts.makeIncompatibile(corner, corner, co0);
    ts.makeIncompatibile(corner, corner, co1);

    return ts;
}

TileSet<ColorRGBi> makeTerrainTileSet()
{
    TileSet<ColorRGBi> ts;

    // g - grass
    // r - rocks
    // d - dirt
    // w - water
    // u - upwards
    // d - downwards
    // ordered clockwise on the side
    enum {
        g, r, d, w,
        grgu, grgd,
        gd, dg,
        wrg, grw
    };

    const int cliff = ts.emplace(
        Tile<ColorRGBi>(loadImage("sample_in/tiles/terrain/cliff_0.png").square(),
            { ByDirection<int>::nesw(g, grgd, g, grgu), ByDirection<int>::nesw(g, grgu, g, grgd) },
            D4SymmetryHelper::closureFromChar('T'), 2.0f)
    );
    ts[cliff][D4Symmetry::Rotation90] = loadImage("sample_in/tiles/terrain/cliff_1.png").square();
    ts[cliff][D4Symmetry::Rotation180] = loadImage("sample_in/tiles/terrain/cliff_2.png").square();
    ts[cliff][D4Symmetry::Rotation270] = loadImage("sample_in/tiles/terrain/cliff_3.png").square();

    const int cliffstairs = ts.emplace(
        Tile<ColorRGBi>(loadImage("sample_in/tiles/terrain/cliffstairs_0.png").square(),
            { ByDirection<int>::nesw(g, grgd, g, grgu), ByDirection<int>::nesw(g, grgu, g, grgd) },
            D4SymmetryHelper::closureFromChar('T'), 0.5f)
    );
    ts[cliffstairs][D4Symmetry::Rotation90] = loadImage("sample_in/tiles/terrain/cliff_1.png").square();
    ts[cliffstairs][D4Symmetry::Rotation180] = loadImage("sample_in/tiles/terrain/cliffstairs_2.png").square();
    ts[cliffstairs][D4Symmetry::Rotation270] = loadImage("sample_in/tiles/terrain/cliff_3.png").square();

    const int cliffcorner = ts.emplace(
        Tile<ColorRGBi>(loadImage("sample_in/tiles/terrain/cliffcorner_0.png").square(),
            { ByDirection<int>::nesw(grgd, grgu, g, g), ByDirection<int>::nesw(grgu, grgd, g, g) },
            D4SymmetryHelper::closureFromChar('L'), 2.0f)
    );
    ts[cliffcorner][D4Symmetry::Rotation90] = loadImage("sample_in/tiles/terrain/cliffcorner_1.png").square();
    ts[cliffcorner][D4Symmetry::Rotation180] = loadImage("sample_in/tiles/terrain/cliffcorner_2.png").square();
    ts[cliffcorner][D4Symmetry::Rotation270] = loadImage("sample_in/tiles/terrain/cliffcorner_3.png").square();

    const int cliffturn = ts.emplace(
        Tile<ColorRGBi>(loadImage("sample_in/tiles/terrain/cliffturn_0.png").square(),
            { ByDirection<int>::nesw(grgu, grgd, g, g), ByDirection<int>::nesw(grgd, grgu, g, g) },
            D4SymmetryHelper::closureFromChar('L'), 2.0f)
    );
    ts[cliffturn][D4Symmetry::Rotation90] = loadImage("sample_in/tiles/terrain/cliffturn_1.png").square();
    ts[cliffturn][D4Symmetry::Rotation180] = loadImage("sample_in/tiles/terrain/cliffturn_2.png").square();
    ts[cliffturn][D4Symmetry::Rotation270] = loadImage("sample_in/tiles/terrain/cliffturn_3.png").square();

    const int grass = ts.emplace(
        Tile<ColorRGBi>(loadImage("sample_in/tiles/terrain/grass_0.png").square(),
            { ByDirection<int>::nesw(g, g, g, g) },
            D4SymmetryHelper::closureFromChar('X'), 8.0f)
    );

    const int grasscorner = ts.emplace(
        Tile<ColorRGBi>(loadImage("sample_in/tiles/terrain/grasscorner_0.png").square(),
            { ByDirection<int>::nesw(dg, gd, d, d), ByDirection<int>::nesw(gd, dg, d, d) },
            D4SymmetryHelper::closureFromChar('L'), 0.0001f)
    );

    const int road = ts.emplace(
        Tile<ColorRGBi>(loadImage("sample_in/tiles/terrain/road_0.png").square(),
            { ByDirection<int>::nesw(d, dg, g, gd), ByDirection<int>::nesw(d, gd, g, dg) },
            D4SymmetryHelper::closureFromChar('T'), 2.0f)
    );
    ts[road][D4Symmetry::Rotation90] = loadImage("sample_in/tiles/terrain/road_1.png").square();
    ts[road][D4Symmetry::Rotation180] = loadImage("sample_in/tiles/terrain/road_2.png").square();
    ts[road][D4Symmetry::Rotation270] = loadImage("sample_in/tiles/terrain/road_3.png").square();

    const int roadturn = ts.emplace(
        Tile<ColorRGBi>(loadImage("sample_in/tiles/terrain/roadturn_0.png").square(),
            { ByDirection<int>::nesw(gd, dg, g, g), ByDirection<int>::nesw(dg, gd, g, g) },
            D4SymmetryHelper::closureFromChar('L'), 0.1f)
    );
    ts[roadturn][D4Symmetry::Rotation90] = loadImage("sample_in/tiles/terrain/roadturn_1.png").square();
    ts[roadturn][D4Symmetry::Rotation180] = loadImage("sample_in/tiles/terrain/roadturn_2.png").square();
    ts[roadturn][D4Symmetry::Rotation270] = loadImage("sample_in/tiles/terrain/roadturn_3.png").square();

    const int water_a = ts.emplace(
        Tile<ColorRGBi>(loadImage("sample_in/tiles/terrain/water_a_0.png").square(),
            { ByDirection<int>::nesw(w, w, w, w), ByDirection<int>::nesw(w, w, w, w) },
            D4SymmetryHelper::closureFromChar('X'), 1.0f)
    );

    const int water_b = ts.emplace(
        Tile<ColorRGBi>(loadImage("sample_in/tiles/terrain/water_b_0.png").square(),
            { ByDirection<int>::nesw(w, w, w, w), ByDirection<int>::nesw(w, w, w, w) },
            D4SymmetryHelper::closureFromChar('X'), 0.5f)
    );

    const int water_c = ts.emplace(
        Tile<ColorRGBi>(loadImage("sample_in/tiles/terrain/water_c_0.png").square(),
            { ByDirection<int>::nesw(w, w, w, w), ByDirection<int>::nesw(w, w, w, w) },
            D4SymmetryHelper::closureFromChar('X'), 0.5f)
    );

    const int watercorner = ts.emplace(
        Tile<ColorRGBi>(loadImage("sample_in/tiles/terrain/watercorner_0.png").square(),
            { ByDirection<int>::nesw(grw, wrg, g, g), ByDirection<int>::nesw(wrg, grw, g, g) },
            D4SymmetryHelper::closureFromChar('L'), 0.5f)
    );
    ts[watercorner][D4Symmetry::Rotation90] = loadImage("sample_in/tiles/terrain/watercorner_1.png").square();
    ts[watercorner][D4Symmetry::Rotation180] = loadImage("sample_in/tiles/terrain/watercorner_2.png").square();
    ts[watercorner][D4Symmetry::Rotation270] = loadImage("sample_in/tiles/terrain/watercorner_3.png").square();

    const int waterside = ts.emplace(
        Tile<ColorRGBi>(loadImage("sample_in/tiles/terrain/waterside_0.png").square(),
            { ByDirection<int>::nesw(w, wrg, g, grw), ByDirection<int>::nesw(w, grw, g, wrg) },
            D4SymmetryHelper::closureFromChar('T'), 0.5f)
    );
    ts[waterside][D4Symmetry::Rotation90] = loadImage("sample_in/tiles/terrain/waterside_1.png").square();
    ts[waterside][D4Symmetry::Rotation180] = loadImage("sample_in/tiles/terrain/waterside_2.png").square();
    ts[waterside][D4Symmetry::Rotation270] = loadImage("sample_in/tiles/terrain/waterside_3.png").square();

    const int waterturn = ts.emplace(
        Tile<ColorRGBi>(loadImage("sample_in/tiles/terrain/waterturn_0.png").square(),
            { ByDirection<int>::nesw(w, w, wrg, grw), ByDirection<int>::nesw(w, w, grw, wrg) },
            D4SymmetryHelper::closureFromChar('L'), 0.5f)
    );
    ts[waterturn][D4Symmetry::Rotation90] = loadImage("sample_in/tiles/terrain/waterturn_1.png").square();
    ts[waterturn][D4Symmetry::Rotation180] = loadImage("sample_in/tiles/terrain/waterturn_2.png").square();
    ts[waterturn][D4Symmetry::Rotation270] = loadImage("sample_in/tiles/terrain/waterturn_3.png").square();

    ts.makeIncompatibile(waterside, waterside, w);
    ts.makeIncompatibile(waterturn, waterturn, w);
    ts.makeIncompatibile(waterside, waterturn, w);

    ts.makeIncompatibile(grasscorner, grasscorner, gd);
    ts.makeIncompatibile(grasscorner, grasscorner, dg);

    ts.makeIncompatibile(cliffstairs, cliffstairs, grgu);
    ts.makeIncompatibile(cliffstairs, cliffstairs, grgd);

    return ts;
}

TileSet<ColorRGBi> makeWangTileSet()
{
    TileSet<ColorRGBi> ts;

    enum {
        r, g, b, w
    };

    const int bbwb = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/wang/bbwb.png").square(), { ByDirection<int>::nesw(b, b, w, b) }, D4SymmetryHelper::closureFromChar('T'), 1.0f, D4Symmetries::None));
    const int brbg = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/wang/brbg.png").square(), { ByDirection<int>::nesw(b, r, b, g) }, D4SymmetryHelper::closureFromChar('C'), 1.0f, D4Symmetries::None));
    const int brwr = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/wang/brwr.png").square(), { ByDirection<int>::nesw(b, r, w, r) }, D4SymmetryHelper::closureFromChar('C'), 1.0f, D4Symmetries::None));
    const int bwbr = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/wang/bwbr.png").square(), { ByDirection<int>::nesw(b, w, b, r) }, D4SymmetryHelper::closureFromChar('T'), 1.0f, D4Symmetries::None));
    const int ggbr = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/wang/ggbr.png").square(), { ByDirection<int>::nesw(g, g, b, r) }, D4SymmetryHelper::closureFromChar('P'), 1.0f, D4Symmetries::None));
    const int rgbw = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/wang/rgbw.png").square(), { ByDirection<int>::nesw(r, g, b, w) }, D4SymmetryHelper::closureFromChar('P'), 1.0f, D4Symmetries::None));
    const int rggg = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/wang/rggg.png").square(), { ByDirection<int>::nesw(r, g, g, g) }, D4SymmetryHelper::closureFromChar('T'), 1.0f, D4Symmetries::None));
    const int rrrg = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/wang/rrrg.png").square(), { ByDirection<int>::nesw(r, r, r, g) }, D4SymmetryHelper::closureFromChar('C'), 1.0f, D4Symmetries::None));
    const int rwrg = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/wang/rwrg.png").square(), { ByDirection<int>::nesw(r, w, r, g) }, D4SymmetryHelper::closureFromChar('C'), 1.0f, D4Symmetries::None));
    const int wbrb = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/wang/wbrb.png").square(), { ByDirection<int>::nesw(w, b, r, b) }, D4SymmetryHelper::closureFromChar('C'), 1.0f, D4Symmetries::None));
    const int wwrw = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/wang/wwrw.png").square(), { ByDirection<int>::nesw(w, w, r, w) }, D4SymmetryHelper::closureFromChar('T'), 1.0f, D4Symmetries::None));

    return ts;
}

double elapsedSeconds(std::chrono::high_resolution_clock::time_point a, std::chrono::high_resolution_clock::time_point b)
{
    return (b - a).count() * 1e-9;
}

double elapsedSeconds(std::chrono::nanoseconds ns)
{
    return ns.count() * 1e-9;
}

void testQueue()
{
    UpdatablePriorityQueue<int> q(16);

    auto print = [](auto& val) { std::cout << val << ' '; };

    q.push(7);
    q.emplace(1);
    q.push(6);
    q.push(3);
    q.push(0);
    auto a = q.push(8);
    auto b = q.push(9);
    q.emplace(4);
    q.push(5);
    q.push(2);

    q.forEach(print);
    std::cout << '\n';

    q.erase(a);

    q.push(321);
    q.update(b, [](auto& v) { v = 123; });

    q.forEach(print);
    std::cout << '\n';

    std::cout << q.top() << '\n';
    q.pop();
    std::cout << q.top() << '\n';

    q.forEach(print);
    std::cout << '\n';

    assert(q.size() == 9);
}

template <typename ModelT>
std::chrono::nanoseconds generateAndSave(ModelT&& model, int count, std::string dir)
{
    auto duration = std::chrono::nanoseconds(0);

    std::filesystem::create_directories(dir.c_str());
    for (int i = 0; i < count; ++i)
    {
        auto t0 = std::chrono::high_resolution_clock::now();
        auto v = model.next();
        auto t1 = std::chrono::high_resolution_clock::now();
        duration += t1 - t0;

        if (v.has_value())
        {
            saveImage(v.value(), dir + "/" + std::to_string(i) + ".png");
            LOG_ERROR(g_logger, "Successful");
        }
        else
        {
            LOG_ERROR(g_logger, "Contradiction");
        }
    }

    LOG_INFO(g_logger, "Time: ", elapsedSeconds(duration));

    return duration;
}

template <typename ModelT>
std::pair<bool, std::chrono::nanoseconds> generateAndSaveOne(ModelT&& model, std::string dir, int idx, int maxTries)
{
    auto duration = std::chrono::nanoseconds(0);

    std::filesystem::create_directories(dir.c_str());
    bool success = false;
    for (int i = 0; i < maxTries; ++i)
    {
        auto t0 = std::chrono::high_resolution_clock::now();
        auto v = model.next();
        auto t1 = std::chrono::high_resolution_clock::now();
        duration += t1 - t0;

        if (v.has_value())
        {
            saveImage(v.value(), dir + "/" + std::to_string(idx) + ".png");
            LOG_ERROR(g_logger, "Successful");
            success = true;
            break;
        }
        else
        {
            LOG_ERROR(g_logger, "Contradiction");
        }
    }

    LOG_INFO(g_logger, "Time: ", elapsedSeconds(duration));

    return { success, duration };
}

std::string toString(Size2i size)
{
    return std::to_string(size.width) + "x" + std::to_string(size.height);
}

std::chrono::nanoseconds generateAndSaveExamples()
{
    using Tiled = TiledModel<ColorRGBi>;
    using TiledOpt = typename TiledModel<ColorRGBi>::OptionsType;
    using Overlapping = OverlappingModel<ColorRGBi>;
    using OverlappingOpt = typename OverlappingModel<ColorRGBi>::OptionsType;

    auto duration = std::chrono::nanoseconds(0);

    for (const auto& size : std::vector<Size2i>{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 }, { 128, 128 }, { 256, 256 } })
    {
        duration += generateAndSave(
            Overlapping(
                loadImage("sample_in/cave.png"),
                OverlappingOpt()
                    .withOutputSize(size)
                    .withOutputWrapping(WrappingMode::None)
                    .withInputWrapping(WrappingMode::None)
                    .withPatternSize(3)
                    .withSymmetries(D4Symmetries::All)
            ),
            32,
            "examples_out/cave/" + toString(size)
        );
    }

    for (const auto& size : std::vector<Size2i>{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 }, { 128, 128 } })
    {
        duration += generateAndSave(
            Overlapping(
                loadImage("sample_in/wireworld.png"),
                OverlappingOpt()
                    .withOutputSize(size)
                    .withOutputWrapping(WrappingMode::None)
                    .withInputWrapping(WrappingMode::None)
                    .withPatternSize(3)
                    .withSymmetries(D4Symmetries::All)
            ),
            32,
            "examples_out/wireworld/" + toString(size)
        );
    }

    for (const auto& size : std::vector<Size2i>{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 }, { 128, 128 }, { 256, 256 } })
    {
        duration += generateAndSave(
            Overlapping(
                loadImage("sample_in/dungeon.png"),
                OverlappingOpt()
                    .withOutputSize(size)
                    .withOutputWrapping(WrappingMode::None)
                    .withInputWrapping(WrappingMode::None)
                    .withPatternSize(3)
                    .withSymmetries(D4Symmetries::All)
            ),
            32,
            "examples_out/dungeon/" + toString(size)
        );
    }

    for (const auto& size : std::vector<Size2i>{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 } })
    {
        duration += generateAndSave(
            Overlapping(
                loadImage("sample_in/penrose.png"),
                OverlappingOpt()
                    .withOutputSize(size)
                    .withOutputWrapping(WrappingMode::None)
                    .withInputWrapping(WrappingMode::None)
                    .withPatternSize(5)
                    .withSymmetries(D4Symmetries::None)
                    .withStride({ 2, 2 })
            ),
            32,
            "examples_out/penrose_p5s2/" + toString(size)
        );
    }

    for (const auto& size : std::vector<Size2i>{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 }, { 128, 128 } })
    {
        for (int i = 0; i < 32; ++i)
        {
            auto [s, d] = generateAndSaveOne(
                Overlapping(
                    i == 0 ? loadImage("sample_in/penrose.png") : loadImage("examples_out/penrose_rec/" + toString(size) + "/" + std::to_string(i-1) + ".png"),
                    OverlappingOpt()
                        .withOutputSize(size)
                        .withOutputWrapping(WrappingMode::None)
                        .withInputWrapping(WrappingMode::None)
                        .withPatternSize(3)
                        .withSymmetries(D4Symmetries::None)
                ),
                "examples_out/penrose_rec/" + toString(size),
                i,
                32
            );

            duration += d;
            if (!s)
            {
                break;
            }
        }
    }

    for (const auto& size : std::vector<Size2i>{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 }, { 128, 128 } })
    {
        duration += generateAndSave(
            Overlapping(
                loadImage("sample_in/penrose.png"),
                OverlappingOpt()
                    .withOutputSize(size)
                    .withOutputWrapping(WrappingMode::None)
                    .withInputWrapping(WrappingMode::None)
                    .withPatternSize(3)
                    .withSymmetries(D4Symmetries::None)
            ),
            32,
            "examples_out/penrose/" + toString(size)
        );
    }

    for (const auto& size : std::vector<Size2i>{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 }, { 128, 128 }, { 256, 256 } })
    {
        duration += generateAndSave(
            Overlapping(
                loadImage("sample_in/maze.png"),
                OverlappingOpt()
                    .withOutputSize(size)
                    .withOutputWrapping(WrappingMode::None)
                    .withInputWrapping(WrappingMode::None)
                    .withPatternSize(3)
                    .withSymmetries(D4Symmetries::None)
            ),
            32,
            "examples_out/maze/" + toString(size)
        );
    }

    for (const std::string s : { "font_upper", "font_lower", "font_digit" })
    {
        for (const auto& size : std::vector<Size2i>{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 }, { 128, 128 } })
        {
            duration += generateAndSave(
                Overlapping(
                    loadImage("sample_in/" + s + ".png"),
                    OverlappingOpt()
                    .withOutputSize(size)
                    .withOutputWrapping(WrappingMode::None)
                    .withInputWrapping(WrappingMode::None)
                    .withPatternSize(3)
                    .withSymmetries(D4Symmetries::None)
                ),
                32,
                "examples_out/" + s + "/" + toString(size)
            );
        }
    }

    for (const std::string s : { "font_upper", "font_lower", "font_digit" })
    {
        for (const auto& size : std::vector<Size2i>{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 }, { 128, 128 } })
        {
            duration += generateAndSave(
                Overlapping(
                    loadImage("sample_in/" + s + ".png"),
                    OverlappingOpt()
                    .withOutputSize(size)
                    .withOutputWrapping(WrappingMode::None)
                    .withInputWrapping(WrappingMode::None)
                    .withPatternSize(3)
                    .withSymmetries(D4Symmetries::All)
                ),
                32,
                "examples_out/" + s + "_sym/" + toString(size)
            );
        }
    }

    for (const auto& size : std::vector<Size2i>{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 } })
    {
        duration += generateAndSave(
            Tiled(
                makeWangTileSet(),
                TiledOpt()
                .withOutputSize(size)
                .withOutputWrapping(WrappingMode::None)
            ),
            32,
            "examples_out/wang/" + toString(size)
        );
    }

    for (const auto& size : std::vector<Size2i>{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 } })
    {
        duration += generateAndSave(
            Tiled(
                makeTerrainTileSet(),
                TiledOpt()
                    .withOutputSize(size)
                    .withOutputWrapping(WrappingMode::None)
            ),
            32,
            "examples_out/terrain/" + toString(size)
        );
    }

    for (const auto& size : std::vector<Size2i>{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 }, { 128, 128 }, { 256, 256 } })
    {
        duration += generateAndSave(
            Tiled(
                makeCircuitTileSet(),
                TiledOpt()
                    .withOutputSize(size)
                    .withOutputWrapping(WrappingMode::All)
            ),
            32,
            "examples_out/circuit/" + toString(size)
        );
    }

    for (const auto& size : std::vector<Size2i>{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 }, { 128, 128 }, { 256, 256 } })
    {
        for (const auto& [subset, name] : std::map<KnotTileSetSubset, std::string>{
            { KnotTileSetSubset::All, "all" },
            { KnotTileSetSubset::Standard, "standard" },
            { KnotTileSetSubset::Dense, "dense" },
            { KnotTileSetSubset::Crossless, "crossless" },
            { KnotTileSetSubset::TE, "te" },
            { KnotTileSetSubset::T, "t" },
            { KnotTileSetSubset::CL, "cl" },
            { KnotTileSetSubset::CE, "ce" },
            { KnotTileSetSubset::C, "c" },
            { KnotTileSetSubset::Fabric, "fabric" },
            { KnotTileSetSubset::DenseFabric, "dense_fabric" }
            })
        {
            duration += generateAndSave(
                Tiled(
                    makeKnotTileSet(subset),
                    TiledOpt()
                        .withOutputSize(size)
                        .withOutputWrapping(WrappingMode::All)
                ),
                32,
                "examples_out/knot/" + name + "/" + toString(size)
            );
        }
    }

    for (const auto& size : std::vector<Size2i>{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 }, { 128, 128 }, { 256, 256 } })
    {
        duration += generateAndSave(
            Overlapping(
                loadImage("sample_in/flowers.png"),
                OverlappingOpt()
                    .withOutputSize(size)
                    .withOutputWrapping(WrappingMode::All)
                    .withInputWrapping(WrappingMode::All)
                    .withPatternSize(3)
                    .withSymmetries(D4Symmetries::All)
            ),
            32,
            "examples_out/flower/" + toString(size)
        );
    }

    return duration;
}

int main()
{
    //testQueue();
    //return 0;

    {
        auto t = generateAndSaveExamples();
        LOG_INFO(g_logger, "Total Time: ", elapsedSeconds(t));
    }

    return 0;
    {
        TiledModelOptions<ColorRGBi> opt;
        opt.outputSize = { 128, 128 };
        opt.outputWrapping = WrappingMode::None;

        auto t0 = std::chrono::high_resolution_clock::now();
        TiledModel<ColorRGBi> m(makeTerrainTileSet(), opt);
        auto t1 = std::chrono::high_resolution_clock::now();

        int i = 0;
        for (auto&& v : m.tryNextN(std::execution::par, 32))
        {
            saveImage(v, std::string("sample_out/terrain/128x128/") + std::to_string(i) + ".png");
            ++i;
        }
        LOG_INFO(g_logger, "Successful: ", i);
        auto t2 = std::chrono::high_resolution_clock::now();

        LOG_INFO(g_logger, "Init time: ", elapsedSeconds(t0, t1));
        LOG_INFO(g_logger, " Gen time: ", elapsedSeconds(t1, t2) / 32.0);
    }
    return 0;
    {
        TiledModelOptions<ColorRGBi> opt;
        opt.outputSize = { 128, 128 };
        opt.outputWrapping = WrappingMode::All;

        auto t0 = std::chrono::high_resolution_clock::now();
        TiledModel<ColorRGBi> m(makeCircuitTileSet(), opt);
        auto t1 = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 32; ++i)
        {
            auto v = m.next();
            if (v.has_value())
            {
                //saveImage(v.value(), std::string("sample_out/circuit/") + std::to_string(i) + ".png");
                LOG_ERROR(g_logger, "Successful");
            }
            else
            {
                LOG_ERROR(g_logger, "Contradiction");
            }
        }
        /*
        int i = 0;
        for (auto&& v : m.tryNextN(std::execution::par, 32))
        {
            //saveImage(v, std::string("sample_out/circuit/") + std::to_string(i) + ".png");
            ++i;
        }
        LOG_INFO(g_logger, "Successful: ", i);
        */
        auto t2 = std::chrono::high_resolution_clock::now();

        LOG_INFO(g_logger, "Init time: ", elapsedSeconds(t0, t1));
        LOG_INFO(g_logger, " Gen time: ", elapsedSeconds(t1, t2) / 32.0);
    }

    {
        TiledModelOptions<ColorRGBi> opt;
        opt.outputSize = { 128, 128 };
        opt.outputWrapping = WrappingMode::All;

        auto t0 = std::chrono::high_resolution_clock::now();
        TiledModel<ColorRGBi> m(makeKnotTileSet(), opt);
        auto t1 = std::chrono::high_resolution_clock::now();

        auto v = m.next();
        v = m.next();
        auto t2 = std::chrono::high_resolution_clock::now();
        if (v.has_value())
        {
            //saveImage(v.value(), "sample_out/knot.png");
        }
        else
        {
            LOG_ERROR(g_logger, "Contradiction");
        }

        LOG_INFO(g_logger, "Init time: ", elapsedSeconds(t0, t1));
        LOG_INFO(g_logger, " Gen time: ", elapsedSeconds(t1, t2)/2.0f);
    }

    {
        auto img = loadImage("sample_in/flowers.png");
        OverlappingModelOptions<ColorRGBi> opt{};
        opt.symmetries = D4Symmetries::All;
        opt.inputWrapping = WrappingMode::All;
        opt.outputWrapping = WrappingMode::All;
        opt.patternSize = 3;
        opt.stride = { 1, 1 };
        opt.setOutputSizeAtLeast({ 128, 128 });
        if (!opt.isValid())
        {
            std::cout << "Invalid config";
            return 1;
        }
        auto t0 = std::chrono::high_resolution_clock::now();
        OverlappingModel<ColorRGBi> m(img, opt);
        auto t1 = std::chrono::high_resolution_clock::now();

        auto v = m.next();
        v = m.next();
        auto t2 = std::chrono::high_resolution_clock::now();
        if (v.has_value())
        {
            //saveImage(v.value(), "sample_out/flowers.png");
        }
        else
        {
            LOG_ERROR(g_logger, "Contradiction");
        }

        LOG_INFO(g_logger, "Init time: ", elapsedSeconds(t0, t1));
        LOG_INFO(g_logger, " Gen time: ", elapsedSeconds(t1, t2) / 2.0f);
    }

    return 0;
}
