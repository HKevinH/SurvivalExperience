#pragma once

namespace game
{
class World;
class InventorySystem;

class GatherSystem
{
public:
    void update(World &world, const InventorySystem &inventorySystem) const;
};
} // namespace game
