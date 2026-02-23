#pragma once

#include "../ecs/EntityManager.hpp"
#include "../../shared/protocol/Messages.hpp"
#include <vector>

namespace game
{
struct DamageEvent
{
    EntityId source{0};
    EntityId target{0};
    int amount{0};
};

class World
{
public:
    EntityManager entities;
    EntityId localPlayerId{0};
    std::uint32_t tick{0};
    float elapsedSeconds{0.f};
    std::vector<DamageEvent> pendingDamage;
    bool interactPressed{false};
    bool craftAxePressed{false};

    void queueDamage(EntityId source, EntityId target, int amount);
};
} // namespace game
