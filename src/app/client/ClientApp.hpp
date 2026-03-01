#pragma once

#include "../../engine/networking/NetworkManager.hpp"
#include "../../engine/resources/ResourceManager.hpp"
#include "../../game/GameSimulation.hpp"
#include "../../game/data/SpriteSheetConfig.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <filesystem>
#include <unordered_map>

namespace app::client
{
class ClientApp
{
public:
    explicit ClientApp(const std::filesystem::path &executablePath);
    void run();

private:
    sf::RenderWindow window_;
    game::GameSimulation simulation_;
    engine::NetworkManager network_;
    engine::ResourceManager resources_;
    std::filesystem::path executablePath_;
    std::filesystem::path executableDir_;
    bool hasPlayerTexture_{false};
    bool hasEnemyTexture_{false};
    std::unordered_map<std::string, game::data::SpriteSheetConfig> spriteConfigs_;
    bool prevInteract_{false};
    bool prevCraft_{false};

    [[nodiscard]] std::filesystem::path resolveAssetPath(const std::string &fileName) const;
    bool loadTextureWithConfig(const std::string &id, const std::string &fileName);
    void processEvents();
    void update(float dt);
    void render();
    void renderHud();
};
} // namespace app::client
