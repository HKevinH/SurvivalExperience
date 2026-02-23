#pragma once

#include "../components/Components.hpp"
#include "../data/GameData.hpp"
#include "../../shared/ids/Types.hpp"
#include <optional>

namespace game
{
class Entity
{
public:
    Entity(EntityId id, data::EntityKind kind)
        : id_(id), kind_(kind)
    {
    }

    [[nodiscard]] EntityId id() const { return id_; }
    [[nodiscard]] data::EntityKind kind() const { return kind_; }
    [[nodiscard]] bool alive() const { return alive_; }
    void destroy() { alive_ = false; }

    std::optional<TransformComponent> transform;
    std::optional<MovementComponent> movement;
    std::optional<HealthComponent> health;
    std::optional<SurvivalStatsComponent> survival;
    std::optional<InventoryComponent> inventory;
    std::optional<GatherableComponent> gatherable;
    std::optional<DamageableComponent> damageable;
    std::optional<AIComponent> ai;
    std::optional<RenderComponent> render;
    std::optional<AnimationComponent> animation;
    std::optional<NetworkReplicationComponent> replication;

private:
    EntityId id_{0};
    data::EntityKind kind_{data::EntityKind::Player};
    bool alive_{true};
};
} // namespace game
