#include "ResourceManager.hpp"

namespace engine
{
bool ResourceManager::loadTexture(const std::string &id, const std::string &path)
{
    sf::Texture texture;
    if (!texture.loadFromFile(path))
        return false;

    textures_[id] = std::move(texture);
    return true;
}

bool ResourceManager::hasTexture(const std::string &id) const
{
    return textures_.contains(id);
}

sf::Texture *ResourceManager::getTexture(const std::string &id)
{
    const auto it = textures_.find(id);
    return it == textures_.end() ? nullptr : &it->second;
}

bool ResourceManager::loadFont(const std::string &id, const std::string &path)
{
    sf::Font font;
    if (!font.openFromFile(path))
        return false;

    fonts_[id] = std::move(font);
    return true;
}

sf::Font *ResourceManager::getFont(const std::string &id)
{
    const auto it = fonts_.find(id);
    return it == fonts_.end() ? nullptr : &it->second;
}
} // namespace engine
