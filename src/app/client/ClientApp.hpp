#pragma once

#include "../../engine/networking/NetworkManager.hpp"
#include "../../engine/resources/ResourceManager.hpp"
#include "../../game/GameSimulation.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <unordered_map>

namespace app::client
{
class ClientApp
{
public:
    ClientApp();
    void run();

private:
    sf::RenderWindow window_;
    game::GameSimulation simulation_;
    engine::NetworkManager network_;
    engine::ResourceManager resources_;
    bool hasPlayerTexture_{false};
    bool hasEnemyTexture_{false};
    std::unordered_map<std::string, sf::IntRect> opaqueSourceRects_;
    bool prevInteract_{false};
    bool prevCraft_{false};

    void processEvents();
    void update(float dt);
    void render();
    void renderHud();
};
} // namespace app::client
