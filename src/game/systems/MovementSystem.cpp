#include "MovementSystem.hpp"

#include "../world/World.hpp"
#include "../../engine/core/Config.hpp"

namespace game
{
void MovementSystem::update(World &world, float dt) const
{
    for (auto &entityPtr : world.entities.all())
    {
        auto &entity = *entityPtr;
        if (!entity.alive() || !entity.transform || !entity.movement)
            continue;

        auto &transform = *entity.transform;
        auto &movement = *entity.movement;

        transform.position.x += movement.velocity.x * dt;
        transform.position.y += movement.velocity.y * dt;

        if (entity.kind() == data::EntityKind::Player)
        {
            const sf::Vector2f half{transform.size.x * 0.5f, transform.size.y * 0.5f};
            if (transform.position.x < half.x)
                transform.position.x = half.x;
            if (transform.position.y < half.y)
                transform.position.y = half.y;
            if (transform.position.x > engine::config::WorldWidth - half.x)
                transform.position.x = engine::config::WorldWidth - half.x;
            if (transform.position.y > engine::config::WorldHeight - half.y)
                transform.position.y = engine::config::WorldHeight - half.y;
        }
    }
}
} // namespace game
