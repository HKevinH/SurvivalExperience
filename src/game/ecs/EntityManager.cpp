#include "EntityManager.hpp"

#include <algorithm>

namespace game
{
Entity &EntityManager::create(data::EntityKind kind)
{
    auto entity = std::make_unique<Entity>(nextId_++, kind);
    Entity &ref = *entity;
    entities_.push_back(std::move(entity));
    return ref;
}

void EntityManager::removeDead()
{
    entities_.erase(
        std::remove_if(
            entities_.begin(),
            entities_.end(),
            [](const std::unique_ptr<Entity> &e)
            {
                return !e->alive();
            }),
        entities_.end());
}

Entity *EntityManager::find(EntityId id)
{
    for (auto &entity : entities_)
    {
        if (entity->id() == id)
            return entity.get();
    }
    return nullptr;
}

const Entity *EntityManager::find(EntityId id) const
{
    for (const auto &entity : entities_)
    {
        if (entity->id() == id)
            return entity.get();
    }
    return nullptr;
}

Entity *EntityManager::firstOf(data::EntityKind kind)
{
    for (auto &entity : entities_)
    {
        if (entity->alive() && entity->kind() == kind)
            return entity.get();
    }
    return nullptr;
}

const Entity *EntityManager::firstOf(data::EntityKind kind) const
{
    for (const auto &entity : entities_)
    {
        if (entity->alive() && entity->kind() == kind)
            return entity.get();
    }
    return nullptr;
}
} // namespace game
