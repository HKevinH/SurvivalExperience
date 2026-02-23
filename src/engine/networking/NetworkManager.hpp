#pragma once

#include "../../shared/protocol/Messages.hpp"
#include <optional>
#include <queue>
#include <string>

namespace engine
{
enum class NetworkMode
{
    Offline,
    Client,
    Server
};

class NetworkManager
{
public:
    explicit NetworkManager(NetworkMode mode = NetworkMode::Offline);

    bool startClient(const std::string &host, unsigned short port);
    bool startServer(unsigned short port);
    void stop();
    void poll();

    void sendInput(const protocol::InputCommand &input);
    void pushSnapshotForTesting(const protocol::Snapshot &snapshot);
    std::optional<protocol::Snapshot> popSnapshot();

    [[nodiscard]] NetworkMode mode() const { return mode_; }
    [[nodiscard]] bool isConnected() const { return connected_; }

private:
    NetworkMode mode_{NetworkMode::Offline};
    bool connected_{false};
    std::queue<protocol::Snapshot> snapshots_;
};
} // namespace engine
