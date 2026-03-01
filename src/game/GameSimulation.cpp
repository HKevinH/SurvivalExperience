#include "GameSimulation.hpp"

#include "../engine/core/Config.hpp"
#include "data/AssetStandards.hpp"
#include "data/GameData.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <cmath>
#include <random>

namespace
{
sf::Color colorFor(const game::Entity &entity)
{
    if (entity.render)
        return entity.render->fill;

    using game::data::EntityKind;
    switch (entity.kind())
    {
    case EntityKind::Player:
        return sf::Color(80, 200, 255);
    case EntityKind::Enemy:
        return sf::Color(220, 70, 70);
    case EntityKind::Tree:
        return sf::Color(50, 130, 60);
    case EntityKind::Rock:
        return sf::Color(110, 110, 120);
    case EntityKind::Bush:
        return sf::Color(70, 160, 80);
    }
    return sf::Color::White;
}
} // namespace

namespace game
{
GameSimulation::GameSimulation() = default;

void GameSimulation::configurePlayerSpriteLayout(const data::SpriteSheetConfig &config)
{
    Entity *player = world_.entities.find(world_.localPlayerId);
    if (player == nullptr || !player->animation || !player->render)
        return;

    sf::Vector2i frameSize{
        std::max(1, config.frameSize.x),
        std::max(1, config.frameSize.y)};
    const int columns = std::max(1, config.columns);
    const int rows = std::max(1, config.rows);

    auto &anim = *player->animation;
    anim.frameSize = frameSize;
    anim.framesPerRow = config.animated ? columns : 1;
    anim.rowIdle = std::clamp(config.idleRow, 0, rows - 1);
    anim.rowWalk = std::clamp(config.walkRow, 0, rows - 1);
    anim.currentRow = anim.rowIdle;
    anim.currentFrame = 0;
    anim.accumulator = 0.f;
    anim.playing = false;

    player->render->sourceRect = config.sourceRect.has_value()
                                     ? config.sourceRect
                                     : std::optional<sf::IntRect>(sf::IntRect({0, anim.rowIdle * frameSize.y}, frameSize));
    player->render->drawSize = {frameSize.x * config.drawScale, frameSize.y * config.drawScale};
}

void GameSimulation::configureEnemySpriteLayout(const data::SpriteSheetConfig &config)
{
    sf::Vector2i frameSize{
        std::max(1, config.frameSize.x),
        std::max(1, config.frameSize.y)};
    const int columns = std::max(1, config.columns);
    const int rows = std::max(1, config.rows);

    for (auto &entityPtr : world_.entities.all())
    {
        Entity &entity = *entityPtr;
        if (!entity.alive() || entity.kind() != data::EntityKind::Enemy || !entity.animation || !entity.render)
            continue;

        auto &anim = *entity.animation;
        anim.frameSize = frameSize;
        anim.framesPerRow = config.animated ? columns : 1;
        anim.rowIdle = std::clamp(config.idleRow, 0, rows - 1);
        anim.rowWalk = std::clamp(config.walkRow, 0, rows - 1);
        anim.currentRow = anim.rowIdle;
        anim.currentFrame = 0;
        anim.accumulator = 0.f;
        anim.playing = false;
        anim.frameTime = 0.16f;

        entity.render->sourceRect = config.sourceRect.has_value()
                                        ? config.sourceRect
                                        : std::optional<sf::IntRect>(sf::IntRect({0, anim.rowIdle * frameSize.y}, frameSize));
        entity.render->drawSize = {frameSize.x * config.drawScale, frameSize.y * config.drawScale};
    }
}

void GameSimulation::initializePrototypeWorld()
{
    world_ = World{};

    Entity &player = world_.entities.create(data::EntityKind::Player);
    // Visual/collision size for current placeholder sprite; tune later per asset.
    player.transform = TransformComponent{{engine::config::WorldWidth * 0.5f, engine::config::WorldHeight * 0.5f}, {52.f, 52.f}};
    player.movement = MovementComponent{{0.f, 0.f}, 180.f};
    player.health = HealthComponent{100, 100};
    player.survival = SurvivalStatsComponent{};
    player.inventory = InventoryComponent{};
    player.render = RenderComponent{
        sf::Color(80, 200, 255),
        sf::Color::White,
        2.f,
        assets::PlayerDrawSize,
        std::nullopt};
    player.render->sourceRect = assets::PlayerDefaultSourceRect;
    // Disabled by default until a real spritesheet is provided/configured.
    // For a valid spritesheet, set frameSize and framesPerRow accordingly.
    player.animation = AnimationComponent{};
    player.animation->frameSize = assets::PlayerFrameSize;
    player.animation->framesPerRow = 1; // sube a >1 cuando uses spritesheet real
    player.replication = NetworkReplicationComponent{1, true};
    world_.localPlayerId = player.id();

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> xDist(80.f, engine::config::WorldWidth - 80.f);
    std::uniform_real_distribution<float> yDist(80.f, engine::config::WorldHeight - 80.f);

    for (int i = 0; i < 15; ++i)
    {
        Entity &tree = world_.entities.create(data::EntityKind::Tree);
        tree.transform = TransformComponent{{xDist(rng), yDist(rng)}, {34.f, 34.f}};
        tree.gatherable = GatherableComponent{data::ResourceType::Wood, 3, 1, 56.f, false};
        tree.render = RenderComponent{
            sf::Color(50, 130, 60),
            sf::Color(30, 70, 30),
            2.f,
            {0.f, 0.f},
            std::nullopt};
    }

    for (int i = 0; i < 10; ++i)
    {
        Entity &rock = world_.entities.create(data::EntityKind::Rock);
        rock.transform = TransformComponent{{xDist(rng), yDist(rng)}, {30.f, 30.f}};
        rock.gatherable = GatherableComponent{data::ResourceType::Stone, 2, 1, 52.f, false};
        rock.render = RenderComponent{
            sf::Color(120, 120, 130),
            sf::Color(70, 70, 80),
            2.f,
            {0.f, 0.f},
            std::nullopt};
    }

    for (int i = 0; i < 12; ++i)
    {
        Entity &bush = world_.entities.create(data::EntityKind::Bush);
        bush.transform = TransformComponent{{xDist(rng), yDist(rng)}, {24.f, 24.f}};
        bush.gatherable = GatherableComponent{data::ResourceType::Fiber, 2, 1, 46.f, false};
        bush.render = RenderComponent{
            sf::Color(70, 170, 80),
            sf::Color(30, 90, 40),
            1.f,
            {0.f, 0.f},
            std::nullopt};
    }

    for (int i = 0; i < 4; ++i)
    {
        Entity &enemy = world_.entities.create(data::EntityKind::Enemy);
        enemy.transform = TransformComponent{{xDist(rng), yDist(rng)}, {26.f, 26.f}};
        enemy.movement = MovementComponent{{0.f, 0.f}, 95.f};
        enemy.health = HealthComponent{30, 30};
        enemy.ai = AIComponent{};
        enemy.damageable = DamageableComponent{7, 0.6f, 0.f};
        enemy.render = RenderComponent{
            sf::Color(220, 70, 70),
            sf::Color(100, 20, 20),
            2.f,
            assets::EnemyDrawSize,
            std::nullopt};
        enemy.render->sourceRect = assets::EnemyDefaultSourceRect;
        enemy.animation = AnimationComponent{};
        enemy.animation->frameSize = assets::EnemyFrameSize;
        enemy.animation->framesPerRow = 1;
        enemy.animation->frameTime = 0.16f;
        enemy.replication = NetworkReplicationComponent{static_cast<NetId>(10 + i), true};
    }
}

void GameSimulation::setLocalMovementInput(float x, float y, bool sprint)
{
    localInputDir_ = {x, y};
    localSprint_ = sprint;
}

void GameSimulation::setLocalActionInput(bool interact, bool craftAxe)
{
    world_.interactPressed = interact;
    world_.craftAxePressed = craftAxe;
}

void GameSimulation::applyLocalPlayerInput()
{
    Entity *player = world_.entities.find(world_.localPlayerId);
    if (player == nullptr || !player->alive() || !player->movement)
        return;

    sf::Vector2f dir = localInputDir_;
    const float lenSq = dir.x * dir.x + dir.y * dir.y;
    if (lenSq <= 0.f)
    {
        player->movement->velocity = {0.f, 0.f};
        return;
    }

    const float len = std::sqrt(lenSq);
    dir = {dir.x / len, dir.y / len};
    const float speedMul = localSprint_ ? 1.45f : 1.f;
    const float speed = player->movement->maxSpeed * speedMul;
    player->movement->velocity = {dir.x * speed, dir.y * speed};

    if (player->survival && localSprint_)
        player->survival->energy = std::max(0.f, player->survival->energy - 0.35f);
}

void GameSimulation::tick(float dt)
{
    world_.elapsedSeconds += dt;
    ++world_.tick;

    applyLocalPlayerInput();

    if (Entity *player = world_.entities.find(world_.localPlayerId); player && player->animation && player->movement)
    {
        auto &anim = *player->animation;
        const auto &vel = player->movement->velocity;
        const bool moving = (vel.x * vel.x + vel.y * vel.y) > 1.f;

        if (anim.frameSize.x <= 0 || anim.frameSize.y <= 0 || anim.framesPerRow <= 1)
        {
            anim.currentFrame = 0;
            anim.accumulator = 0.f;
            anim.playing = false;
        }
        else
        {
            anim.currentRow = moving ? anim.rowWalk : anim.rowIdle;
            anim.playing = moving;

            if (!moving)
            {
                anim.currentFrame = 0;
                anim.accumulator = 0.f;
            }
            else if (anim.frameTime > 0.f)
            {
                anim.accumulator += dt;
                while (anim.accumulator >= anim.frameTime)
                {
                    anim.accumulator -= anim.frameTime;
                    anim.currentFrame = (anim.currentFrame + 1) % anim.framesPerRow;
                }
            }
        }
    }

    aiSystem_.update(world_, dt);
    movementSystem_.update(world_, dt);

    for (auto &entityPtr : world_.entities.all())
    {
        Entity &entity = *entityPtr;
        if (!entity.alive() || entity.kind() != data::EntityKind::Enemy || !entity.animation || !entity.movement)
            continue;

        auto &anim = *entity.animation;
        const auto &vel = entity.movement->velocity;
        const bool moving = (vel.x * vel.x + vel.y * vel.y) > 1.f;

        if (anim.frameSize.x <= 0 || anim.frameSize.y <= 0 || anim.framesPerRow <= 1)
        {
            anim.currentFrame = 0;
            anim.accumulator = 0.f;
            anim.playing = false;
            continue;
        }

        anim.currentRow = moving ? anim.rowWalk : anim.rowIdle;
        anim.playing = moving;

        if (!moving)
        {
            anim.currentFrame = 0;
            anim.accumulator = 0.f;
        }
        else if (anim.frameTime > 0.f)
        {
            anim.accumulator += dt;
            while (anim.accumulator >= anim.frameTime)
            {
                anim.accumulator -= anim.frameTime;
                anim.currentFrame = (anim.currentFrame + 1) % anim.framesPerRow;
            }
        }
    }

    collisionSystem_.update(world_, dt);
    damageSystem_.update(world_);
    gatherSystem_.update(world_, inventorySystem_);
    craftingSystem_.update(world_, inventorySystem_);
    survivalSystem_.update(world_, dt);
    world_.entities.removeDead();

    world_.interactPressed = false;
    world_.craftAxePressed = false;
}

void GameSimulation::renderDebug(sf::RenderTarget &target, bool drawPlayerDebug, bool drawEnemyDebug) const
{
    sf::RectangleShape floor({engine::config::WorldWidth, engine::config::WorldHeight});
    floor.setPosition({0.f, 0.f});
    floor.setFillColor(sf::Color(28, 53, 32));
    target.draw(floor);

    for (const auto &entityPtr : world_.entities.all())
    {
        const Entity &entity = *entityPtr;
        if (!entity.alive() || !entity.transform)
            continue;

        if (!drawPlayerDebug && entity.kind() == data::EntityKind::Player)
            continue;
        if (!drawEnemyDebug && entity.kind() == data::EntityKind::Enemy)
            continue;

        sf::RectangleShape shape(entity.transform->size);
        shape.setOrigin({entity.transform->size.x * 0.5f, entity.transform->size.y * 0.5f});
        shape.setPosition(entity.transform->position);
        shape.setFillColor(colorFor(entity));
        if (entity.render)
        {
            shape.setOutlineColor(entity.render->outline);
            shape.setOutlineThickness(entity.render->outlineThickness);
        }
        target.draw(shape);
    }
}

protocol::Snapshot GameSimulation::buildSnapshot() const
{
    protocol::Snapshot snapshot;
    snapshot.serverTick = world_.tick;

    for (const auto &entityPtr : world_.entities.all())
    {
        const Entity &entity = *entityPtr;
        if (!entity.alive() || !entity.transform)
            continue;

        protocol::ReplicatedEntityState state;
        state.netId = entity.replication ? entity.replication->netId : entity.id();
        state.type = static_cast<std::uint8_t>(entity.kind());
        state.position = {entity.transform->position.x, entity.transform->position.y};
        if (entity.movement)
            state.velocity = {entity.movement->velocity.x, entity.movement->velocity.y};
        if (entity.health)
            state.health = static_cast<std::int16_t>(entity.health->current);
        snapshot.entities.push_back(state);
    }

    return snapshot;
}
} // namespace game
