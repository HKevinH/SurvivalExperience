#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

namespace engine
{
class Animation
{
public:
    Animation(sf::Sprite &sprite,
              const sf::Texture &texture,
              int frameWidth,
              int frameHeight,
              float frameDurationSeconds = 0.1f);

    void update(float deltaTimeSeconds);

    void setRow(int row);
    void playRow(int row, int startColumn = 0, int frameCount = -1, bool loop = true);
    void setFrame(int frameIndexInCurrentRow);
    void reset();
    void play();
    void pause();

    [[nodiscard]] int columns() const { return columns_; }
    [[nodiscard]] int rows() const { return rows_; }
    [[nodiscard]] int currentRow() const { return currentRow_; }
    [[nodiscard]] int currentFrame() const { return currentFrame_; }
    [[nodiscard]] sf::Vector2i frameSize() const { return frameSize_; }

private:
    sf::Sprite &sprite_;
    const sf::Texture &texture_;
    sf::Vector2i frameSize_{0, 0};

    int columns_{1};
    int rows_{1};
    int currentRow_{0};
    int currentFrame_{0};
    int startColumn_{0};
    int activeFrameCount_{1};
    float frameDuration_{0.1f};
    float accumulator_{0.f};
    bool playing_{true};
    bool looping_{true};

    void applyTextureRect();
    [[nodiscard]] int clampedRow(int row) const;
    [[nodiscard]] int clampedColumn(int column) const;
};
} // namespace engine
