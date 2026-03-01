#include "ClientApp.hpp"

#include "../../engine/core/Config.hpp"
#include "../../game/data/AssetStandards.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <iostream>
#include <vector>

namespace app::client
{
namespace
{
game::data::SpriteSheetConfig defaultConfigFor(const std::string &id, sf::Vector2u textureSize)
{
    using game::data::SpriteSheetConfig;
    SpriteSheetConfig cfg{};
    if (id == "player")
    {
        cfg.frameSize = game::assets::PlayerFrameSize;
        cfg.columns = 0;
        cfg.rows = 0;
        cfg.idleRow = 0;
        cfg.walkRow = 1;
        cfg.drawScale = game::assets::PlayerScale;
        cfg.animated = true;
    }
    else
    {
        cfg.frameSize = game::assets::EnemyFrameSize;
        cfg.columns = 0;
        cfg.rows = 0;
        cfg.idleRow = 0;
        cfg.walkRow = 1;
        cfg.drawScale = game::assets::EnemyScale;
        cfg.animated = true;
    }

    std::string err;
    if (!game::data::validateAndFinalizeSpriteSheetConfig(cfg, textureSize, err))
    {
        cfg = SpriteSheetConfig{};
        cfg.frameSize = {static_cast<int>(textureSize.x), static_cast<int>(textureSize.y)};
        cfg.columns = 1;
        cfg.rows = 1;
        cfg.idleRow = 0;
        cfg.walkRow = 0;
        cfg.drawScale = (id == "player") ? game::assets::PlayerScale : game::assets::EnemyScale;
        cfg.animated = false;
    }

    return cfg;
}
} // namespace

ClientApp::ClientApp(const std::filesystem::path &executablePath)
    : window_(sf::VideoMode({engine::config::WindowWidth, engine::config::WindowHeight}), "Mini Survival Coop Prototype"),
      network_(engine::NetworkMode::Client),
      executablePath_(executablePath),
      executableDir_(std::filesystem::absolute(executablePath).parent_path())
{
    window_.setFramerateLimit(60);
    simulation_.initializePrototypeWorld();

    hasPlayerTexture_ = loadTextureWithConfig("player", "player.png");
    hasEnemyTexture_ = loadTextureWithConfig("enemy", "enemy.png");

    if (!hasPlayerTexture_)
        std::cout << "[Client] player.png no se pudo cargar.\n";
    if (!hasEnemyTexture_)
        std::cout << "[Client] enemy.png no se pudo cargar.\n";

    if (hasPlayerTexture_)
    {
        const auto &cfg = spriteConfigs_.at("player");
        simulation_.configurePlayerSpriteLayout(cfg);
    }
    if (hasEnemyTexture_)
    {
        const auto &cfg = spriteConfigs_.at("enemy");
        simulation_.configureEnemySpriteLayout(cfg);
    }

    network_.startClient("127.0.0.1", 7777);
}

std::filesystem::path ClientApp::resolveAssetPath(const std::string &fileName) const
{
    const std::vector<std::filesystem::path> candidates{
        executableDir_ / "assets" / fileName,
        executableDir_.parent_path() / "src" / "assets" / fileName,
        executableDir_.parent_path() / "assets" / fileName,
        std::filesystem::current_path() / "src" / "assets" / fileName,
        std::filesystem::current_path() / "assets" / fileName};

    for (const auto &path : candidates)
    {
        if (std::filesystem::exists(path))
            return path;
    }

    return {};
}

bool ClientApp::loadTextureWithConfig(const std::string &id, const std::string &fileName)
{
    const std::filesystem::path texPath = resolveAssetPath(fileName);
    if (texPath.empty())
        return false;

    if (!resources_.loadTexture(id, texPath.string()))
        return false;

    sf::Texture *texture = resources_.getTexture(id);
    if (texture == nullptr)
        return false;

    game::data::SpriteSheetConfig cfg{};
    const std::filesystem::path cfgPath = texPath.parent_path() / (texPath.stem().string() + ".spritecfg");
    const bool hasExternalCfg = game::data::loadSpriteSheetConfig(cfgPath.string(), cfg);
    if (!hasExternalCfg)
        cfg = defaultConfigFor(id, texture->getSize());

    std::string err;
    if (!game::data::validateAndFinalizeSpriteSheetConfig(cfg, texture->getSize(), err))
    {
        std::cout << "[Client] Config invalida en " << cfgPath << " -> " << err << ". Usando fallback.\n";
        cfg = defaultConfigFor(id, texture->getSize());
    }

    spriteConfigs_[id] = cfg;
    return true;
}

void ClientApp::run()
{
    sf::Clock clock;
    while (window_.isOpen())
    {
        const float dt = std::min(0.05f, clock.restart().asSeconds());
        processEvents();
        update(dt);
        render();
    }
}

void ClientApp::processEvents()
{
    while (const auto event = window_.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            window_.close();
    }
}

void ClientApp::update(float dt)
{
    float x = 0.f;
    float y = 0.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        x += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        y += 1.f;

    const bool sprint = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift);
    const bool interactNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E);
    const bool craftNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C);
    const bool interactPressed = interactNow && !prevInteract_;
    const bool craftPressed = craftNow && !prevCraft_;
    prevInteract_ = interactNow;
    prevCraft_ = craftNow;

    simulation_.setLocalMovementInput(x, y, sprint);
    simulation_.setLocalActionInput(interactPressed, craftPressed);

    protocol::InputCommand input{};
    input.moveX = x;
    input.moveY = y;
    input.sprint = sprint;
    input.interact = interactPressed;
    input.craftAxe = craftPressed;
    network_.sendInput(input);
    network_.poll();

    simulation_.tick(dt);
    network_.pushSnapshotForTesting(simulation_.buildSnapshot());
    (void)network_.popSnapshot();
}

void ClientApp::render()
{
    window_.clear(sf::Color(15, 15, 20));
    simulation_.renderDebug(window_, !hasPlayerTexture_, !hasEnemyTexture_);

    auto drawEntitySprite = [this](const game::Entity &entity, const char *textureId)
    {
        if (!entity.transform)
            return;

        sf::Texture *texture = resources_.getTexture(textureId);
        if (texture == nullptr)
            return;

        sf::Sprite sprite(*texture);
        const auto texSize = texture->getSize();
        sf::Vector2f baseSize(static_cast<float>(texSize.x), static_cast<float>(texSize.y));

        if (entity.animation && entity.animation->frameSize.x > 0 && entity.animation->frameSize.y > 0 &&
            entity.animation->framesPerRow > 1)
        {
            const auto &anim = *entity.animation;
            const sf::IntRect rect(
                {anim.currentFrame * anim.frameSize.x, anim.currentRow * anim.frameSize.y},
                anim.frameSize);
            sprite.setTextureRect(rect);
            baseSize = {static_cast<float>(rect.size.x), static_cast<float>(rect.size.y)};
        }
        else if (entity.render && entity.render->sourceRect.has_value())
        {
            const sf::IntRect rect = *entity.render->sourceRect;
            sprite.setTextureRect(rect);
            baseSize = {static_cast<float>(rect.size.x), static_cast<float>(rect.size.y)};
        }

        sprite.setOrigin({baseSize.x * 0.5f, baseSize.y * 0.5f});
        sf::Vector2f targetSize = entity.transform->size;
        if (entity.render && entity.render->drawSize.x > 0.f && entity.render->drawSize.y > 0.f)
            targetSize = entity.render->drawSize;

        const float scale = std::min(targetSize.x / baseSize.x, targetSize.y / baseSize.y);
        sprite.setScale({scale, scale});
        sprite.setPosition(entity.transform->position);
        window_.draw(sprite);
    };

    const game::Entity *player = simulation_.world().entities.find(simulation_.world().localPlayerId);
    if (hasPlayerTexture_ && player && player->transform)
        drawEntitySprite(*player, "player");

    if (hasEnemyTexture_)
    {
        for (const auto &entityPtr : simulation_.world().entities.all())
        {
            const game::Entity &entity = *entityPtr;
            if (!entity.alive() || entity.kind() != game::data::EntityKind::Enemy)
                continue;
            drawEntitySprite(entity, "enemy");
        }
    }

    renderHud();
    window_.display();
}

void ClientApp::renderHud()
{
    const game::Entity *player = simulation_.world().entities.find(simulation_.world().localPlayerId);
    if (player == nullptr)
        return;

    sf::RectangleShape panel({340.f, 120.f});
    panel.setPosition({12.f, 12.f});
    panel.setFillColor(sf::Color(0, 0, 0, 150));
    panel.setOutlineColor(sf::Color(220, 220, 220, 180));
    panel.setOutlineThickness(1.f);
    window_.draw(panel);

    auto drawBar = [this](sf::Vector2f pos, sf::Color color, float value)
    {
        sf::RectangleShape bg({220.f, 14.f});
        bg.setPosition(pos);
        bg.setFillColor(sf::Color(40, 40, 40, 220));
        window_.draw(bg);

        const float clamped = std::clamp(value, 0.f, 100.f);
        sf::RectangleShape fg({220.f * (clamped / 100.f), 14.f});
        fg.setPosition(pos);
        fg.setFillColor(color);
        window_.draw(fg);
    };

    float health = player->health ? static_cast<float>(player->health->current) : 0.f;
    float hunger = player->survival ? player->survival->hunger : 0.f;
    float energy = player->survival ? player->survival->energy : 0.f;
    drawBar({24.f, 24.f}, sf::Color(210, 60, 60), health);
    drawBar({24.f, 46.f}, sf::Color(220, 170, 60), hunger);
    drawBar({24.f, 68.f}, sf::Color(70, 180, 220), energy);
}
} // namespace app::client
