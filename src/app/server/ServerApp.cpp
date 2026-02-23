#include "ServerApp.hpp"

#include "../../engine/core/Config.hpp"
#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>
#include <chrono>
#include <iostream>

namespace app::server
{
ServerApp::ServerApp()
    : network_(engine::NetworkMode::Server)
{
    simulation_.initializePrototypeWorld();
    network_.startServer(7777);
}

void ServerApp::run()
{
    using namespace std::chrono_literals;

    sf::Clock clock;
    float accumulator = 0.f;
    const float fixedDt = 1.f / engine::config::ServerTickRate;
    std::uint32_t lastPrintedSecond = 0;

    std::cout << "[Server] Running authoritative simulation on port 7777 (stub transport)\n";

    while (true)
    {
        const float dt = std::min(0.25f, clock.restart().asSeconds());
        accumulator += dt;
        network_.poll();

        while (accumulator >= fixedDt)
        {
            simulation_.setLocalMovementInput(0.f, 0.f, false);
            simulation_.setLocalActionInput(false, false);
            simulation_.tick(fixedDt);
            accumulator -= fixedDt;
        }

        const auto &world = simulation_.world();
        const std::uint32_t sec = static_cast<std::uint32_t>(world.elapsedSeconds);
        if (sec != lastPrintedSecond)
        {
            lastPrintedSecond = sec;
            const game::Entity *player = world.entities.find(world.localPlayerId);
            const int hp = (player && player->health) ? player->health->current : 0;
            std::cout << "[Server] t=" << sec
                      << "s tick=" << world.tick
                      << " entities=" << world.entities.all().size()
                      << " playerHP=" << hp << '\n';
        }

        sf::sleep(sf::milliseconds(1));
    }
}
} // namespace app::server
