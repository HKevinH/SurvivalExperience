#include "SpriteSheetConfig.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string_view>
#include <unordered_map>

namespace
{
std::string trim(std::string s)
{
    auto notSpace = [](unsigned char c)
    { return !std::isspace(c); };

    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

bool parseInt(const std::string &text, int &out)
{
    try
    {
        std::size_t idx = 0;
        const long value = std::stol(text, &idx, 10);
        if (idx != text.size())
            return false;
        out = static_cast<int>(value);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool parseFloat(const std::string &text, float &out)
{
    try
    {
        std::size_t idx = 0;
        out = std::stof(text, &idx);
        return idx == text.size();
    }
    catch (...)
    {
        return false;
    }
}

bool parseBool(const std::string &text, bool &out)
{
    if (text == "1" || text == "true" || text == "TRUE")
    {
        out = true;
        return true;
    }
    if (text == "0" || text == "false" || text == "FALSE")
    {
        out = false;
        return true;
    }
    return false;
}
} // namespace

namespace game::data
{
bool loadSpriteSheetConfig(const std::string &path, SpriteSheetConfig &outConfig)
{
    std::ifstream file(path);
    if (!file.is_open())
        return false;

    std::unordered_map<std::string, std::string> kv;
    std::string line;
    while (std::getline(file, line))
    {
        const auto hashPos = line.find('#');
        if (hashPos != std::string::npos)
            line = line.substr(0, hashPos);

        line = trim(line);
        if (line.empty())
            continue;

        const auto eqPos = line.find('=');
        if (eqPos == std::string::npos)
            continue;

        std::string key = trim(line.substr(0, eqPos));
        std::string value = trim(line.substr(eqPos + 1));
        if (!key.empty())
            kv[key] = value;
    }

    SpriteSheetConfig cfg{};
    int v = 0;
    float f = 0.f;
    bool b = false;

    if (kv.contains("frame_w") && parseInt(kv["frame_w"], v))
        cfg.frameSize.x = v;
    if (kv.contains("frame_h") && parseInt(kv["frame_h"], v))
        cfg.frameSize.y = v;
    if (kv.contains("columns") && parseInt(kv["columns"], v))
        cfg.columns = v;
    if (kv.contains("rows") && parseInt(kv["rows"], v))
        cfg.rows = v;
    if (kv.contains("idle_row") && parseInt(kv["idle_row"], v))
        cfg.idleRow = v;
    if (kv.contains("walk_row") && parseInt(kv["walk_row"], v))
        cfg.walkRow = v;
    if (kv.contains("draw_scale") && parseFloat(kv["draw_scale"], f))
        cfg.drawScale = f;
    if (kv.contains("animated") && parseBool(kv["animated"], b))
        cfg.animated = b;

    int sx = 0, sy = 0, sw = 0, sh = 0;
    const bool hasRect =
        kv.contains("source_x") && kv.contains("source_y") &&
        kv.contains("source_w") && kv.contains("source_h") &&
        parseInt(kv["source_x"], sx) && parseInt(kv["source_y"], sy) &&
        parseInt(kv["source_w"], sw) && parseInt(kv["source_h"], sh);
    if (hasRect)
        cfg.sourceRect = sf::IntRect({sx, sy}, {sw, sh});

    outConfig = cfg;
    return true;
}

bool validateAndFinalizeSpriteSheetConfig(
    SpriteSheetConfig &config,
    sf::Vector2u textureSize,
    std::string &outError)
{
    const int texW = static_cast<int>(textureSize.x);
    const int texH = static_cast<int>(textureSize.y);
    if (texW <= 0 || texH <= 0)
    {
        outError = "Texture size is zero.";
        return false;
    }

    if (config.frameSize.x <= 0 || config.frameSize.y <= 0)
    {
        config.frameSize = {texW, texH};
        config.columns = 1;
        config.rows = 1;
        config.animated = false;
    }

    if (config.columns <= 0)
        config.columns = std::max(1, texW / std::max(1, config.frameSize.x));
    if (config.rows <= 0)
        config.rows = std::max(1, texH / std::max(1, config.frameSize.y));

    if (config.frameSize.x * config.columns > texW || config.frameSize.y * config.rows > texH)
    {
        outError = "Configured frame grid exceeds texture dimensions.";
        return false;
    }

    config.idleRow = std::clamp(config.idleRow, 0, config.rows - 1);
    config.walkRow = std::clamp(config.walkRow, 0, config.rows - 1);
    config.drawScale = std::max(0.1f, config.drawScale);

    if (config.sourceRect.has_value())
    {
        const auto &r = *config.sourceRect;
        const bool valid =
            r.position.x >= 0 && r.position.y >= 0 &&
            r.size.x > 0 && r.size.y > 0 &&
            r.position.x + r.size.x <= texW &&
            r.position.y + r.size.y <= texH;
        if (!valid)
        {
            outError = "Configured sourceRect is out of texture bounds.";
            return false;
        }
    }

    if (!config.animated || config.columns <= 1)
    {
        config.animated = false;
        config.columns = 1;
    }

    return true;
}
} // namespace game::data
