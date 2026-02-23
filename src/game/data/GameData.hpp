#pragma once

#include "../../shared/ids/Types.hpp"
#include <array>
#include <string_view>

namespace game::data
{
enum class EntityKind : std::uint8_t
{
    Player,
    Enemy,
    Tree,
    Rock,
    Bush
};

enum class ResourceType : std::uint8_t
{
    None,
    Wood,
    Stone,
    Fiber
};

enum class ToolType : std::uint8_t
{
    None,
    Axe,
    Spear
};

enum class ItemKind : ItemId
{
    Wood = 1,
    Stone = 2,
    Fiber = 3,
    StoneAxe = 4,
    Spear = 5
};

struct ItemDefinition
{
    ItemKind id;
    std::string_view name;
    int maxStack;
};

inline constexpr std::array<ItemDefinition, 5> Items{{
    {ItemKind::Wood, "Wood", 50},
    {ItemKind::Stone, "Stone", 50},
    {ItemKind::Fiber, "Fiber", 50},
    {ItemKind::StoneAxe, "Stone Axe", 1},
    {ItemKind::Spear, "Spear", 1},
}};

inline constexpr int findMaxStack(ItemKind id)
{
    for (const auto &item : Items)
    {
        if (item.id == id)
            return item.maxStack;
    }
    return 1;
}
} // namespace game::data
