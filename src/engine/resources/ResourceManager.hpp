#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <string>
#include <unordered_map>

namespace engine
{
class ResourceManager
{
public:
    bool loadTexture(const std::string &id, const std::string &path);
    bool hasTexture(const std::string &id) const;
    sf::Texture *getTexture(const std::string &id);

    bool loadFont(const std::string &id, const std::string &path);
    sf::Font *getFont(const std::string &id);

private:
    std::unordered_map<std::string, sf::Texture> textures_;
    std::unordered_map<std::string, sf::Font> fonts_;
};
} // namespace engine
