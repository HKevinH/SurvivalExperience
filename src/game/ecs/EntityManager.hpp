#pragma once

#include "Entity.hpp"
#include <memory>
#include <vector>

namespace game
{
class EntityManager
{
public:
    Entity &create(data::EntityKind kind);
    void removeDead();

    [[nodiscard]] Entity *find(EntityId id);
    [[nodiscard]] const Entity *find(EntityId id) const;
    [[nodiscard]] Entity *firstOf(data::EntityKind kind);
    [[nodiscard]] const Entity *firstOf(data::EntityKind kind) const;

    [[nodiscard]] std::vector<std::unique_ptr<Entity>> &all() { return entities_; }
    [[nodiscard]] const std::vector<std::unique_ptr<Entity>> &all() const { return entities_; }

private:
    EntityId nextId_{1};
    std::vector<std::unique_ptr<Entity>> entities_;
};
} // namespace game
