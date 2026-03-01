#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>
#include <string>

namespace game::data
{
struct SpriteSheetConfig
{
    sf::Vector2i frameSize{0, 0};
    int columns{0}; // 0 = auto from texture width/frameSize.x
    int rows{0};    // 0 = auto from texture height/frameSize.y
    int idleRow{0};
    int walkRow{0};
    float drawScale{1.f};
    bool animated{false};
    std::optional<sf::IntRect> sourceRect;
};

bool loadSpriteSheetConfig(const std::string &path, SpriteSheetConfig &outConfig);
bool validateAndFinalizeSpriteSheetConfig(
    SpriteSheetConfig &config,
    sf::Vector2u textureSize,
    std::string &outError);
} // namespace game::data
