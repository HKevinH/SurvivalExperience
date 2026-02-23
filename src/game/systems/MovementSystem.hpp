#pragma once

namespace game
{
class World;

class MovementSystem
{
public:
    void update(World &world, float dt) const;
};
} // namespace game
