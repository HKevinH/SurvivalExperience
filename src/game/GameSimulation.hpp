#pragma once

#include "systems/AISystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/CraftingSystem.hpp"
#include "data/SpriteSheetConfig.hpp"
#include "systems/DamageSystem.hpp"
#include "systems/GatherSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/SurvivalSystem.hpp"
#include "world/World.hpp"
#include "../shared/protocol/Messages.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

namespace game
{
class GameSimulation
{
public:
    GameSimulation();

    void initializePrototypeWorld();
    void configurePlayerSpriteLayout(const data::SpriteSheetConfig &config);
    void configureEnemySpriteLayout(const data::SpriteSheetConfig &config);
    void setLocalMovementInput(float x, float y, bool sprint);
    void setLocalActionInput(bool interact, bool craftAxe);
    void tick(float dt);
    void renderDebug(sf::RenderTarget &target, bool drawPlayerDebug = true, bool drawEnemyDebug = true) const;

    [[nodiscard]] const World &world() const { return world_; }
    [[nodiscard]] World &world() { return world_; }
    [[nodiscard]] protocol::Snapshot buildSnapshot() const;

private:
    World world_;
    MovementSystem movementSystem_;
    SurvivalSystem survivalSystem_;
    AISystem aiSystem_;
    CollisionSystem collisionSystem_;
    DamageSystem damageSystem_;
    InventorySystem inventorySystem_;
    GatherSystem gatherSystem_;
    CraftingSystem craftingSystem_;
    sf::Vector2f localInputDir_{0.f, 0.f};
    bool localSprint_{false};

    void applyLocalPlayerInput();
};
} // namespace game
