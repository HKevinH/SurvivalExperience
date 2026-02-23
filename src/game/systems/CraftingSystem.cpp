#include "CraftingSystem.hpp"

#include "InventorySystem.hpp"
#include "../world/World.hpp"

namespace game
{
void CraftingSystem::update(World &world, const InventorySystem &inventorySystem) const
{
    if (!world.craftAxePressed)
        return;

    Entity *player = world.entities.find(world.localPlayerId);
    if (player == nullptr || !player->alive() || !player->inventory)
        return;

    using data::ItemKind;
    const bool hasMaterials =
        inventorySystem.countItem(*player, ItemKind::Wood) >= 2 &&
        inventorySystem.countItem(*player, ItemKind::Stone) >= 1 &&
        inventorySystem.countItem(*player, ItemKind::Fiber) >= 1;

    if (!hasMaterials)
        return;

    if (!inventorySystem.consumeItem(*player, ItemKind::Wood, 2))
        return;
    if (!inventorySystem.consumeItem(*player, ItemKind::Stone, 1))
        return;
    if (!inventorySystem.consumeItem(*player, ItemKind::Fiber, 1))
        return;

    inventorySystem.addItem(*player, ItemKind::StoneAxe, 1);
}
} // namespace game
