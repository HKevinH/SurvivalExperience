#include "InventorySystem.hpp"

#include "../ecs/Entity.hpp"

namespace game
{
bool InventorySystem::addItem(Entity &entity, data::ItemKind item, int count) const
{
    if (!entity.inventory || count <= 0)
        return false;

    auto &inv = *entity.inventory;
    const int maxStack = data::findMaxStack(item);

    for (auto &slot : inv.slots)
    {
        if (count <= 0)
            break;
        if (!slot.occupied || slot.item != item || slot.count >= maxStack)
            continue;

        const int add = std::min(count, maxStack - slot.count);
        slot.count += add;
        count -= add;
    }

    for (auto &slot : inv.slots)
    {
        if (count <= 0)
            break;
        if (slot.occupied)
            continue;

        slot.occupied = true;
        slot.item = item;
        slot.count = std::min(count, maxStack);
        count -= slot.count;
    }

    return count == 0;
}

bool InventorySystem::consumeItem(Entity &entity, data::ItemKind item, int count) const
{
    if (!entity.inventory || count <= 0 || countItem(entity, item) < count)
        return false;

    auto &inv = *entity.inventory;
    for (auto &slot : inv.slots)
    {
        if (count <= 0)
            break;
        if (!slot.occupied || slot.item != item)
            continue;

        const int take = std::min(count, slot.count);
        slot.count -= take;
        count -= take;

        if (slot.count == 0)
        {
            slot.occupied = false;
        }
    }

    return true;
}

int InventorySystem::countItem(const Entity &entity, data::ItemKind item) const
{
    if (!entity.inventory)
        return 0;

    int total = 0;
    for (const auto &slot : entity.inventory->slots)
    {
        if (slot.occupied && slot.item == item)
            total += slot.count;
    }
    return total;
}
} // namespace game
