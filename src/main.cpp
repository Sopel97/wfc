#include <iostream>

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

TileSet<ColorRGBi> makeKnotTileSet()
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

    return ts;
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

double elapsedSeconds(std::chrono::high_resolution_clock::time_point a, std::chrono::high_resolution_clock::time_point b)
{
    return (b - a).count() * 1e-9;
}

int main()
{
    {
        TiledModelOptions opt;
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
        auto t2 = std::chrono::high_resolution_clock::now();

        LOG_INFO(g_logger, "Init time: ", elapsedSeconds(t0, t1));
        LOG_INFO(g_logger, " Gen time: ", elapsedSeconds(t1, t2) / 32.0);
    }

    {
        TiledModelOptions opt;
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
        OverlappingModelOptions opt{};
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
