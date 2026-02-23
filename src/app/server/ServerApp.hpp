#pragma once

#include "../../engine/networking/NetworkManager.hpp"
#include "../../game/GameSimulation.hpp"

namespace app::server
{
class ServerApp
{
public:
    ServerApp();
    void run();

private:
    game::GameSimulation simulation_;
    engine::NetworkManager network_;
};
} // namespace app::server
