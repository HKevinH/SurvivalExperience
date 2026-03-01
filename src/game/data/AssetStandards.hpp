#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace game::assets
{
    // Proyecto: estándar configurable para pixel-art.
    // Cambia estas constantes para pasar de 16x16 a 32x32 sin tocar lógica.
    inline constexpr int StandardFramePixels = 16;    // opción recomendada MVP
    inline constexpr int AlternativeFramePixels = 32; // opción alternativa

    inline constexpr sf::Vector2i PlayerFrameSize{StandardFramePixels, StandardFramePixels};
    inline constexpr sf::Vector2i EnemyFrameSize{StandardFramePixels, StandardFramePixels};

    // Escalas visuales separadas de la colisión (pixel-perfect upscaling).
    inline constexpr float PlayerScale = 3.0f; // 16x16 -> 48x48
    inline constexpr float EnemyScale = 1.5f;  // 16x16 -> 40x40 aprox

    inline constexpr sf::Vector2f PlayerDrawSize{
        static_cast<float>(PlayerFrameSize.x) * PlayerScale,
        static_cast<float>(PlayerFrameSize.y) * PlayerScale};

    inline constexpr sf::Vector2f EnemyDrawSize{
        static_cast<float>(EnemyFrameSize.x) * EnemyScale,
        static_cast<float>(EnemyFrameSize.y) * EnemyScale};

    inline constexpr sf::IntRect PlayerDefaultSourceRect{{0, 0}, PlayerFrameSize};
    inline constexpr sf::IntRect EnemyDefaultSourceRect{{0, 0}, EnemyFrameSize};
} // namespace game::assets
