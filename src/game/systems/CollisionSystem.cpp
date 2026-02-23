#include "CollisionSystem.hpp"

#include "../world/World.hpp"

namespace game
{
void CollisionSystem::update(World &world, float dt) const
{
    for (auto &entityPtr : world.entities.all())
    {
        auto &entity = *entityPtr;
        if (!entity.alive() || !entity.damageable)
            continue;

        entity.damageable->hitCooldownRemaining -= dt;
        if (entity.damageable->hitCooldownRemaining < 0.f)
            entity.damageable->hitCooldownRemaining = 0.f;
    }

    auto &entities = world.entities.all();
    for (std::size_t i = 0; i < entities.size(); ++i)
    {
        auto &a = *entities[i];
        if (!a.alive() || !a.transform)
            continue;

        for (std::size_t j = i + 1; j < entities.size(); ++j)
        {
            auto &b = *entities[j];
            if (!b.alive() || !b.transform)
                continue;

            const auto hit = a.transform->bounds().findIntersection(b.transform->bounds());
            if (!hit.has_value())
                continue;

            if (a.kind() == data::EntityKind::Enemy && b.kind() == data::EntityKind::Player &&
                a.damageable && a.damageable->hitCooldownRemaining <= 0.f)
            {
                world.queueDamage(a.id(), b.id(), a.damageable->contactDamage);
                a.damageable->hitCooldownRemaining = a.damageable->hitCooldown;
            }

            if (b.kind() == data::EntityKind::Enemy && a.kind() == data::EntityKind::Player &&
                b.damageable && b.damageable->hitCooldownRemaining <= 0.f)
            {
                world.queueDamage(b.id(), a.id(), b.damageable->contactDamage);
                b.damageable->hitCooldownRemaining = b.damageable->hitCooldown;
            }
        }
    }
}
} // namespace game
