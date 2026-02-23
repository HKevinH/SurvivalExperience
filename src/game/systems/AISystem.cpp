#include "AISystem.hpp"

#include "../world/World.hpp"
#include <cmath>

namespace
{
sf::Vector2f normalized(const sf::Vector2f &v)
{
    const float lenSq = v.x * v.x + v.y * v.y;
    if (lenSq <= 0.f)
        return {0.f, 0.f};
    const float len = std::sqrt(lenSq);
    return {v.x / len, v.y / len};
}
} // namespace

namespace game
{
void AISystem::update(World &world, float dt) const
{
    (void)dt;
    Entity *player = world.entities.find(world.localPlayerId);
    if (player == nullptr || !player->alive() || !player->transform)
        return;

    const sf::Vector2f playerPos = player->transform->position;

    for (auto &entityPtr : world.entities.all())
    {
        auto &entity = *entityPtr;
        if (!entity.alive() || !entity.ai || !entity.transform || !entity.movement)
            continue;
        if (entity.kind() != data::EntityKind::Enemy)
            continue;

        auto &ai = *entity.ai;
        auto &transform = *entity.transform;
        auto &movement = *entity.movement;

        const sf::Vector2f delta{playerPos.x - transform.position.x, playerPos.y - transform.position.y};
        const float distSq = delta.x * delta.x + delta.y * delta.y;

        ai.targetPosition = playerPos;
        ai.hasTarget = true;

        if (distSq <= ai.attackRange * ai.attackRange)
        {
            ai.state = AIState::Attack;
            movement.velocity = {0.f, 0.f};
        }
        else if (distSq <= ai.aggroRange * ai.aggroRange)
        {
            ai.state = AIState::Chase;
            const sf::Vector2f dir = normalized(delta);
            movement.velocity = {dir.x * movement.maxSpeed, dir.y * movement.maxSpeed};
        }
        else
        {
            ai.state = AIState::Idle;
            movement.velocity = {0.f, 0.f};
        }
    }
}
} // namespace game
