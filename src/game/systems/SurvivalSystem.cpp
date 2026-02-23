#include "SurvivalSystem.hpp"

#include "../world/World.hpp"
#include <algorithm>

namespace game
{
void SurvivalSystem::update(World &world, float dt) const
{
    for (auto &entityPtr : world.entities.all())
    {
        auto &entity = *entityPtr;
        if (!entity.alive() || !entity.survival || !entity.health)
            continue;

        auto &survival = *entity.survival;
        auto &health = *entity.health;

        survival.hunger = std::max(0.f, survival.hunger - survival.hungerDrainPerSecond * dt);
        survival.energy = std::max(0.f, survival.energy - survival.energyDrainPerSecond * dt);

        if ((survival.hunger <= 0.01f || survival.energy <= 0.01f) && health.current > 0)
        {
            health.current = std::max(0, health.current - 1);
            if (health.current == 0)
                entity.destroy();
        }
    }
}
} // namespace game
