#pragma once

#include "../data/GameData.hpp"

namespace game
{
class Entity;

class InventorySystem
{
public:
    bool addItem(Entity &entity, data::ItemKind item, int count) const;
    bool consumeItem(Entity &entity, data::ItemKind item, int count) const;
    [[nodiscard]] int countItem(const Entity &entity, data::ItemKind item) const;
};
} // namespace game
