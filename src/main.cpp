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

static inline Array2<ColorRGBi> loadImage(const std::string& path)
{
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
    std::vector<unsigned char> data;
    data.reserve(image.size().total() * 3);

    const auto [width, height] = image.size();
    
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

    const int corner = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/knot/corner.png").square(), ByDirection<int>::nesw(p, p, e, e), D4SymmetryHelper::closureFromChar('L'), 1.0f));
    const int cross = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/knot/cross.png").square(), ByDirection<int>::nesw(p, p, p, p), D4SymmetryHelper::closureFromChar('I'), 1.0f));
    const int empty = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/knot/empty.png").square(), ByDirection<int>::nesw(e, e, e, e), D4SymmetryHelper::closureFromChar('X'), 1.0f));
    const int line = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/knot/line.png").square(), ByDirection<int>::nesw(e, p, e, p), D4SymmetryHelper::closureFromChar('I'), 1.0f));
    const int t = ts.emplace(Tile<ColorRGBi>(loadImage("sample_in/tiles/knot/t.png").square(), ByDirection<int>::nesw(e, p, p, p), D4SymmetryHelper::closureFromChar('T'), 1.0f));

    return ts;
}

int main()
{
    {
        TiledModelOptions opt;
        opt.outputSize = { 96, 96 };
        opt.outputWrapping = WrappingMode::All;
        TiledModel<ColorRGBi> m(makeKnotTileSet(), opt);

        auto v = m.next();
        v = m.next();
        if (v.has_value())
        {
            saveImage(v.value(), "sample_out/knot.png");
        }
        else
        {
            std::cout << "contradiction\n";
            return 2;
        }
    }

    {
        auto img = loadImage("sample_in/flowers.png");
        OverlappingModelOptions opt{};
        opt.symmetries = D4Symmetries::All;
        opt.inputWrapping = WrappingMode::All;
        opt.outputWrapping = WrappingMode::All;
        opt.patternSize = 3;
        opt.stride = { 1, 1 };
        opt.setOutputSizeAtLeast({ 96, 96 });
        if (!opt.isValid())
        {
            std::cout << "Invalid config\n";
            return 1;
        }
        OverlappingModel<ColorRGBi> m(img, opt);
        auto v = m.next();
        v = m.next();
        if (v.has_value())
        {
            saveImage(v.value(), "sample_out/flowers.png");
        }
        else
        {
            std::cout << "contradiction\n";
            return 2;
        }
    }

    return 0;
}
