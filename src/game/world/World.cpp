#include "World.hpp"

namespace game
{
void World::queueDamage(EntityId source, EntityId target, int amount)
{
    if (amount <= 0)
        return;
    pendingDamage.push_back({source, target, amount});
}
} // namespace game
