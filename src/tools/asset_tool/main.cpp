#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

struct Options
{
    std::string input;
    std::string output;
    bool trim = false;
    bool hasFrame = false;
    unsigned int frameW = 0;
    unsigned int frameH = 0;
    unsigned int frameX = 0;
    unsigned int frameY = 0;
    bool hasScale = false;
    unsigned int scale = 1;
    unsigned int alphaThreshold = 1;
};

void printUsage()
{
    std::cout
        << "asset_tool - PNG prep for sprites/tiles\n"
        << "Usage:\n"
        << "  asset_tool --in <input.png> --out <output.png> [options]\n\n"
        << "Options:\n"
        << "  --trim                     Trim transparent padding\n"
        << "  --alpha-threshold <0-255>  Alpha threshold for trim (default 1)\n"
        << "  --frame <w> <h> <x> <y>    Extract frame at grid cell (x,y) from spritesheet\n"
        << "  --scale <n>                Nearest-neighbor upscale factor (integer)\n"
        << "\nExamples:\n"
        << "  asset_tool --in src/assets/enemy.png --out src/assets/enemy_frame.png --frame 16 16 0 0 --scale 2\n"
        << "  asset_tool --in src/assets/player.png --out src/assets/player_trim.png --trim\n";
}

bool parseUInt(const std::string &s, unsigned int &out)
{
    try
    {
        std::size_t idx = 0;
        const unsigned long value = std::stoul(s, &idx, 10);
        if (idx != s.size())
            return false;
        out = static_cast<unsigned int>(value);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

std::optional<Options> parseArgs(int argc, char **argv)
{
    Options opt;

    std::vector<std::string> args;
    args.reserve(static_cast<std::size_t>(argc > 0 ? argc - 1 : 0));
    for (int i = 1; i < argc; ++i)
        args.emplace_back(argv[i]);

    for (std::size_t i = 0; i < args.size(); ++i)
    {
        const std::string &a = args[i];
        if (a == "--in" && i + 1 < args.size())
        {
            opt.input = args[++i];
        }
        else if (a == "--out" && i + 1 < args.size())
        {
            opt.output = args[++i];
        }
        else if (a == "--trim")
        {
            opt.trim = true;
        }
        else if (a == "--alpha-threshold" && i + 1 < args.size())
        {
            if (!parseUInt(args[++i], opt.alphaThreshold))
                return std::nullopt;
            opt.alphaThreshold = std::min(255u, opt.alphaThreshold);
        }
        else if (a == "--scale" && i + 1 < args.size())
        {
            if (!parseUInt(args[++i], opt.scale))
                return std::nullopt;
            opt.scale = std::max(1u, opt.scale);
            opt.hasScale = true;
        }
        else if (a == "--frame" && i + 4 < args.size())
        {
            if (!parseUInt(args[++i], opt.frameW) ||
                !parseUInt(args[++i], opt.frameH) ||
                !parseUInt(args[++i], opt.frameX) ||
                !parseUInt(args[++i], opt.frameY))
            {
                return std::nullopt;
            }
            opt.hasFrame = true;
        }
        else if (a == "--help" || a == "-h")
        {
            printUsage();
            std::exit(0);
        }
        else
        {
            return std::nullopt;
        }
    }

    if (opt.input.empty() || opt.output.empty())
        return std::nullopt;

    return opt;
}

std::optional<sf::IntRect> computeOpaqueBounds(const sf::Image &image, unsigned int alphaThreshold)
{
    const sf::Vector2u size = image.getSize();
    if (size.x == 0 || size.y == 0)
        return std::nullopt;

    unsigned int minX = size.x;
    unsigned int minY = size.y;
    unsigned int maxX = 0;
    unsigned int maxY = 0;
    bool found = false;

    for (unsigned int y = 0; y < size.y; ++y)
    {
        for (unsigned int x = 0; x < size.x; ++x)
        {
            const sf::Color px = image.getPixel({x, y});
            if (px.a < alphaThreshold)
                continue;

            found = true;
            minX = std::min(minX, x);
            minY = std::min(minY, y);
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);
        }
    }

    if (!found)
        return std::nullopt;

    return sf::IntRect(
        {static_cast<int>(minX), static_cast<int>(minY)},
        {static_cast<int>(maxX - minX + 1), static_cast<int>(maxY - minY + 1)});
}

bool rectFits(const sf::IntRect &rect, const sf::Vector2u &size)
{
    return rect.position.x >= 0 &&
           rect.position.y >= 0 &&
           rect.size.x > 0 &&
           rect.size.y > 0 &&
           rect.position.x + rect.size.x <= static_cast<int>(size.x) &&
           rect.position.y + rect.size.y <= static_cast<int>(size.y);
}

sf::Image cropImage(const sf::Image &src, const sf::IntRect &rect)
{
    sf::Image out({static_cast<unsigned int>(rect.size.x), static_cast<unsigned int>(rect.size.y)}, sf::Color::Transparent);
    for (int y = 0; y < rect.size.y; ++y)
    {
        for (int x = 0; x < rect.size.x; ++x)
        {
            out.setPixel({static_cast<unsigned int>(x), static_cast<unsigned int>(y)},
                         src.getPixel({static_cast<unsigned int>(rect.position.x + x),
                                       static_cast<unsigned int>(rect.position.y + y)}));
        }
    }
    return out;
}

sf::Image scaleNearest(const sf::Image &src, unsigned int factor)
{
    if (factor <= 1)
        return src;

    const sf::Vector2u srcSize = src.getSize();
    sf::Image out({srcSize.x * factor, srcSize.y * factor}, sf::Color::Transparent);

    for (unsigned int y = 0; y < out.getSize().y; ++y)
    {
        for (unsigned int x = 0; x < out.getSize().x; ++x)
        {
            const unsigned int sx = x / factor;
            const unsigned int sy = y / factor;
            out.setPixel({x, y}, src.getPixel({sx, sy}));
        }
    }

    return out;
}

int main(int argc, char **argv)
{
    const auto parsed = parseArgs(argc, argv);
    if (!parsed.has_value())
    {
        printUsage();
        return 1;
    }
    const Options opt = *parsed;

    sf::Image image;
    if (!image.loadFromFile(opt.input))
    {
        std::cerr << "Error: no se pudo cargar " << opt.input << '\n';
        return 2;
    }

    sf::Image result = image;
    const sf::Vector2u originalSize = image.getSize();

    if (opt.hasFrame)
    {
        const sf::IntRect frameRect(
            {static_cast<int>(opt.frameX * opt.frameW), static_cast<int>(opt.frameY * opt.frameH)},
            {static_cast<int>(opt.frameW), static_cast<int>(opt.frameH)});

        if (!rectFits(frameRect, result.getSize()))
        {
            std::cerr << "Error: frame fuera de rango. Texture=" << result.getSize().x << "x" << result.getSize().y
                      << " frameRect=(" << frameRect.position.x << "," << frameRect.position.y
                      << "," << frameRect.size.x << "," << frameRect.size.y << ")\n";
            return 3;
        }

        result = cropImage(result, frameRect);
    }

    if (opt.trim)
    {
        if (const auto opaqueRect = computeOpaqueBounds(result, opt.alphaThreshold))
        {
            result = cropImage(result, *opaqueRect);
        }
        else
        {
            std::cerr << "Warning: imagen totalmente transparente; trim omitido.\n";
        }
    }

    if (opt.hasScale && opt.scale > 1)
        result = scaleNearest(result, opt.scale);

    if (const fs::path outPath(opt.output); outPath.has_parent_path())
        fs::create_directories(outPath.parent_path());

    if (!result.saveToFile(opt.output))
    {
        std::cerr << "Error: no se pudo guardar " << opt.output << '\n';
        return 4;
    }

    std::cout << "OK\n";
    std::cout << "Input:  " << opt.input << " (" << originalSize.x << "x" << originalSize.y << ")\n";
    std::cout << "Output: " << opt.output << " (" << result.getSize().x << "x" << result.getSize().y << ")\n";
    return 0;
}
