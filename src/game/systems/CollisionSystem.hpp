#pragma once

namespace game
{
class World;

class CollisionSystem
{
public:
    void update(World &world, float dt) const;
};
} // namespace game
