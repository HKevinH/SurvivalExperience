#include "ClientApp.hpp"

#include "../../engine/core/Config.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <iostream>

namespace app::client
{
namespace
{
std::optional<sf::IntRect> computeOpaqueBounds(const sf::Texture &texture)
{
    const sf::Image image = texture.copyToImage();
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
            if (px.a == 0)
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
} // namespace

ClientApp::ClientApp()
    : window_(sf::VideoMode({engine::config::WindowWidth, engine::config::WindowHeight}), "Mini Survival Coop Prototype"),
      network_(engine::NetworkMode::Client)
{
    window_.setFramerateLimit(60);
    simulation_.initializePrototypeWorld();
    hasPlayerTexture_ =
        resources_.loadTexture("player", "src/assets/player.png") ||
        resources_.loadTexture("player", "assets/player.png") ||
        resources_.loadTexture("player", "build/assets/player.png");
    if (!hasPlayerTexture_)
        std::cout << "[Client] No se pudo cargar player sprite (src/assets/player.png)\n";
    else if (sf::Texture *playerTex = resources_.getTexture("player"))
    {
        if (auto rect = computeOpaqueBounds(*playerTex))
            opaqueSourceRects_["player"] = *rect;
    }

    hasEnemyTexture_ =
        resources_.loadTexture("enemy", "src/assets/enemy.png") ||
        resources_.loadTexture("enemy", "src/assets/enemies/enemy.png") ||
        resources_.loadTexture("enemy", "assets/enemy.png") ||
        resources_.loadTexture("enemy", "assets/enemies/enemy.png") ||
        resources_.loadTexture("enemy", "build/assets/enemy.png") ||
        resources_.loadTexture("enemy", "build/assets/enemies/enemy.png");
    if (!hasEnemyTexture_)
        std::cout << "[Client] No se pudo cargar enemy sprite (esperado en src/assets/enemy.png o src/assets/enemies/enemy.png)\n";
    else if (sf::Texture *enemyTex = resources_.getTexture("enemy"))
    {
        if (auto rect = computeOpaqueBounds(*enemyTex))
            opaqueSourceRects_["enemy"] = *rect;
    }

    network_.startClient("127.0.0.1", 7777);
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

        if (sf::Texture *texture = resources_.getTexture(textureId))
        {
            sf::Sprite sprite(*texture);
            const auto texSize = texture->getSize();

            sf::Vector2f baseSize(static_cast<float>(texSize.x), static_cast<float>(texSize.y));
            bool appliedRect = false;
            if (entity.animation && entity.animation->frameSize.x > 0 && entity.animation->frameSize.y > 0)
            {
                const auto &anim = *entity.animation;
                const int frameX = anim.currentFrame * anim.frameSize.x;
                const int frameY = anim.currentRow * anim.frameSize.y;
                const bool validRect =
                    frameX >= 0 && frameY >= 0 &&
                    anim.frameSize.x > 0 && anim.frameSize.y > 0 &&
                    frameX + anim.frameSize.x <= static_cast<int>(texSize.x) &&
                    frameY + anim.frameSize.y <= static_cast<int>(texSize.y);

                if (validRect)
                {
                    sprite.setTextureRect(sf::IntRect({frameX, frameY}, anim.frameSize));
                    baseSize = {static_cast<float>(anim.frameSize.x), static_cast<float>(anim.frameSize.y)};
                    appliedRect = true;
                }
            }

            if (!appliedRect && entity.render && entity.render->sourceRect.has_value())
            {
                const sf::IntRect rect = *entity.render->sourceRect;
                const bool validRect =
                    rect.position.x >= 0 && rect.position.y >= 0 &&
                    rect.size.x > 0 && rect.size.y > 0 &&
                    rect.position.x + rect.size.x <= static_cast<int>(texSize.x) &&
                    rect.position.y + rect.size.y <= static_cast<int>(texSize.y);

                if (validRect)
                {
                    sprite.setTextureRect(rect);
                    baseSize = {static_cast<float>(rect.size.x), static_cast<float>(rect.size.y)};
                    appliedRect = true;
                }
            }

            if (!appliedRect)
            {
                const auto it = opaqueSourceRects_.find(textureId);
                if (it != opaqueSourceRects_.end())
                {
                    sprite.setTextureRect(it->second);
                    baseSize = {static_cast<float>(it->second.size.x), static_cast<float>(it->second.size.y)};
                }
            }

            if (baseSize.x > 0.f && baseSize.y > 0.f)
            {
                sprite.setOrigin({baseSize.x * 0.5f, baseSize.y * 0.5f});
                sf::Vector2f targetSize = entity.transform->size;
                if (entity.render && entity.render->drawSize.x > 0.f && entity.render->drawSize.y > 0.f)
                    targetSize = entity.render->drawSize;
                const float scale = std::min(targetSize.x / baseSize.x, targetSize.y / baseSize.y);
                sprite.setScale({scale, scale});
            }

            sprite.setPosition(entity.transform->position);
            window_.draw(sprite);
        }
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

    if (player->inventory)
    {
        for (std::size_t i = 0; i < 8; ++i)
        {
            sf::RectangleShape slot({32.f, 32.f});
            slot.setPosition({24.f + static_cast<float>(i) * 38.f, 92.f});
            slot.setFillColor(sf::Color(70, 70, 80, 220));
            slot.setOutlineColor(sf::Color::White);
            slot.setOutlineThickness(i == static_cast<std::size_t>(player->inventory->selectedSlot) ? 2.f : 1.f);
            window_.draw(slot);

            if (player->inventory->slots[i].occupied)
            {
                sf::RectangleShape icon({18.f, 18.f});
                icon.setPosition({31.f + static_cast<float>(i) * 38.f, 99.f});
                using game::data::ItemKind;
                switch (player->inventory->slots[i].item)
                {
                case ItemKind::Wood:
                    icon.setFillColor(sf::Color(139, 94, 60));
                    break;
                case ItemKind::Stone:
                    icon.setFillColor(sf::Color(130, 130, 140));
                    break;
                case ItemKind::Fiber:
                    icon.setFillColor(sf::Color(100, 180, 100));
                    break;
                case ItemKind::StoneAxe:
                    icon.setFillColor(sf::Color(180, 180, 220));
                    break;
                case ItemKind::Spear:
                    icon.setFillColor(sf::Color(210, 180, 130));
                    break;
                }
                window_.draw(icon);
            }
        }
    }
}
} // namespace app::client
