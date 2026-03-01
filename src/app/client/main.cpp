#include "ClientApp.hpp"
#include <filesystem>

int main(int argc, char **argv)
{
    (void)argc;
    const std::filesystem::path executablePath = (argv != nullptr && argv[0] != nullptr) ? argv[0] : "";
    app::client::ClientApp app(executablePath);
    app.run();
    return 0;
}
