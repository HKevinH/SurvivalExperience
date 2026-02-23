#pragma once

namespace game
{
class World;

class AISystem
{
public:
    void update(World &world, float dt) const;
};
} // namespace game
