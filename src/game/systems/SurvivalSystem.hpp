#pragma once

namespace game
{
class World;

class SurvivalSystem
{
public:
    void update(World &world, float dt) const;
};
} // namespace game
