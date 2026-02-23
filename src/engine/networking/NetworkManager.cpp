#include "NetworkManager.hpp"

namespace engine
{
NetworkManager::NetworkManager(NetworkMode mode)
    : mode_(mode)
{
}

bool NetworkManager::startClient(const std::string &, unsigned short)
{
    mode_ = NetworkMode::Client;
    connected_ = true;
    return true;
}

bool NetworkManager::startServer(unsigned short)
{
    mode_ = NetworkMode::Server;
    connected_ = true;
    return true;
}

void NetworkManager::stop()
{
    connected_ = false;
    while (!snapshots_.empty())
        snapshots_.pop();
    mode_ = NetworkMode::Offline;
}

void NetworkManager::poll()
{
    // Stub transport. Replace with ENet backend while preserving this API.
}

void NetworkManager::sendInput(const protocol::InputCommand &)
{
    // Stub: in offline/local test mode we do nothing.
}

void NetworkManager::pushSnapshotForTesting(const protocol::Snapshot &snapshot)
{
    snapshots_.push(snapshot);
}

std::optional<protocol::Snapshot> NetworkManager::popSnapshot()
{
    if (snapshots_.empty())
        return std::nullopt;

    protocol::Snapshot snapshot = snapshots_.front();
    snapshots_.pop();
    return snapshot;
}
} // namespace engine
