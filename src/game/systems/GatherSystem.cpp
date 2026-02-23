#include "GatherSystem.hpp"

#include "InventorySystem.hpp"
#include "../world/World.hpp"
#include <cmath>

namespace
{
game::data::ItemKind toItem(game::data::ResourceType r)
{
    using namespace game::data;
    switch (r)
    {
    case ResourceType::Wood:
        return ItemKind::Wood;
    case ResourceType::Stone:
        return ItemKind::Stone;
    case ResourceType::Fiber:
        return ItemKind::Fiber;
    default:
        return ItemKind::Wood;
    }
}
} // namespace

namespace game
{
void GatherSystem::update(World &world, const InventorySystem &inventorySystem) const
{
    if (!world.interactPressed)
        return;

    Entity *player = world.entities.find(world.localPlayerId);
    if (player == nullptr || !player->alive() || !player->transform || !player->inventory)
        return;

    const auto playerPos = player->transform->position;

    for (auto &entityPtr : world.entities.all())
    {
        Entity &node = *entityPtr;
        if (!node.alive() || !node.gatherable || !node.transform)
            continue;

        auto &g = *node.gatherable;
        if (g.depleted)
            continue;

        const sf::Vector2f delta{
            node.transform->position.x - playerPos.x,
            node.transform->position.y - playerPos.y};
        const float distSq = delta.x * delta.x + delta.y * delta.y;
        if (distSq > g.interactRange * g.interactRange)
            continue;

        inventorySystem.addItem(*player, toItem(g.resourceType), g.yieldPerHit);
        g.remainingHits -= 1;
        if (g.remainingHits <= 0)
        {
            g.depleted = true;
            node.destroy();
        }
        break;
    }
}
} // namespace game
