#pragma once

#include "../ids/Types.hpp"
#include "../math/Vec2.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace protocol
{
enum class MessageType : std::uint16_t
{
    Hello = 1,
    Welcome,
    InputCommand,
    Snapshot,
    DamageEvent,
    InventoryDelta,
    GatherRequest,
    CraftRequest
};

struct PacketHeader
{
    std::uint16_t protocolVersion{1};
    MessageType msgType{MessageType::Hello};
    std::uint32_t sequence{0};
    std::uint32_t tick{0};
};

struct InputCommand
{
    std::uint32_t inputSeq{0};
    float moveX{0.f};
    float moveY{0.f};
    bool sprint{false};
    bool interact{false};
    bool craftAxe{false};
};

struct ReplicatedEntityState
{
    NetId netId{0};
    std::uint8_t type{0};
    Vec2 position{};
    Vec2 velocity{};
    std::uint16_t stateFlags{0};
    std::int16_t health{0};
};

struct Snapshot
{
    std::uint32_t serverTick{0};
    std::vector<ReplicatedEntityState> entities;
};
} // namespace protocol
