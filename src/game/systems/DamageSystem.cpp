#include "DamageSystem.hpp"

#include "../world/World.hpp"
#include <algorithm>

namespace game
{
void DamageSystem::update(World &world) const
{
    for (const auto &evt : world.pendingDamage)
    {
        Entity *target = world.entities.find(evt.target);
        if (target == nullptr || !target->alive() || !target->health)
            continue;

        target->health->current = std::max(0, target->health->current - evt.amount);
        if (target->health->current == 0)
            target->destroy();
    }

    world.pendingDamage.clear();
}
} // namespace game
