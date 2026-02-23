#pragma once

#include "../data/GameData.hpp"
#include "../../shared/ids/Types.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <array>
#include <optional>

namespace game
{
struct TransformComponent
{
    sf::Vector2f position{0.f, 0.f};
    sf::Vector2f size{16.f, 16.f};

    [[nodiscard]] sf::FloatRect bounds() const
    {
        return sf::FloatRect({position.x - size.x * 0.5f, position.y - size.y * 0.5f}, size);
    }
};

struct MovementComponent
{
    sf::Vector2f velocity{0.f, 0.f};
    float maxSpeed{0.f};
};

struct HealthComponent
{
    int current{100};
    int max{100};
};

struct SurvivalStatsComponent
{
    float hunger{100.f};
    float energy{100.f};
    float hungerDrainPerSecond{0.35f};
    float energyDrainPerSecond{0.15f};
};

struct InventorySlot
{
    data::ItemKind item{data::ItemKind::Wood};
    int count{0};
    bool occupied{false};
};

struct InventoryComponent
{
    static constexpr std::size_t SlotCount = 16;
    std::array<InventorySlot, SlotCount> slots{};
    int selectedSlot{0};
};

struct GatherableComponent
{
    data::ResourceType resourceType{data::ResourceType::None};
    int remainingHits{3};
    int yieldPerHit{1};
    float interactRange{64.f};
    bool depleted{false};
};

struct DamageableComponent
{
    int contactDamage{5};
    float hitCooldown{0.5f};
    float hitCooldownRemaining{0.f};
};

enum class AIState
{
    Idle,
    Chase,
    Attack
};

struct AIComponent
{
    AIState state{AIState::Idle};
    float aggroRange{220.f};
    float attackRange{28.f};
    sf::Vector2f targetPosition{0.f, 0.f};
    bool hasTarget{false};
};

struct RenderComponent
{
    sf::Color fill{sf::Color::White};
    sf::Color outline{sf::Color::Black};
    float outlineThickness{0.f};
    sf::Vector2f drawSize{0.f, 0.f}; // Visual size; if zero, fallback to transform size.
    std::optional<sf::IntRect> sourceRect; // Optional manual crop in texture space.
};

struct AnimationComponent
{
    sf::Vector2i frameSize{0, 0};
    int framesPerRow{1};
    int rowIdle{0};
    int rowWalk{0};
    int currentRow{0};
    int currentFrame{0};
    float frameTime{0.12f};
    float accumulator{0.f};
    bool playing{false};
};

struct NetworkReplicationComponent
{
    NetId netId{0};
    bool dirty{true};
};
} // namespace game
