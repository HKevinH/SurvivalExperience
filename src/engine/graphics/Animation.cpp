#include "Animation.hpp"

#include <algorithm>

namespace engine
{
Animation::Animation(sf::Sprite &sprite,
                     const sf::Texture &texture,
                     int frameWidth,
                     int frameHeight,
                     float frameDurationSeconds)
    : sprite_(sprite),
      texture_(texture),
      frameSize_{std::max(1, frameWidth), std::max(1, frameHeight)},
      frameDuration_(std::max(0.001f, frameDurationSeconds))
{
    sprite_.setTexture(texture_);

    const sf::Vector2u texSize = texture_.getSize();
    columns_ = std::max(1, static_cast<int>(texSize.x) / frameSize_.x);
    rows_ = std::max(1, static_cast<int>(texSize.y) / frameSize_.y);
    activeFrameCount_ = columns_;

    applyTextureRect();
}

void Animation::update(float deltaTimeSeconds)
{
    if (!playing_ || activeFrameCount_ <= 1)
        return;

    accumulator_ += std::max(0.f, deltaTimeSeconds);
    while (accumulator_ >= frameDuration_)
    {
        accumulator_ -= frameDuration_;
        ++currentFrame_;

        if (currentFrame_ >= activeFrameCount_)
        {
            if (looping_)
            {
                currentFrame_ = 0;
            }
            else
            {
                currentFrame_ = activeFrameCount_ - 1;
                playing_ = false;
            }
        }

        applyTextureRect();
    }
}

void Animation::setRow(int row)
{
    currentRow_ = clampedRow(row);
    currentFrame_ = 0;
    startColumn_ = 0;
    activeFrameCount_ = columns_;
    accumulator_ = 0.f;
    applyTextureRect();
}

void Animation::playRow(int row, int startColumn, int frameCount, bool loop)
{
    currentRow_ = clampedRow(row);
    startColumn_ = clampedColumn(startColumn);
    activeFrameCount_ = frameCount <= 0 ? (columns_ - startColumn_) : frameCount;
    activeFrameCount_ = std::clamp(activeFrameCount_, 1, columns_ - startColumn_);
    currentFrame_ = 0;
    accumulator_ = 0.f;
    looping_ = loop;
    playing_ = true;
    applyTextureRect();
}

void Animation::setFrame(int frameIndexInCurrentRow)
{
    currentFrame_ = std::clamp(frameIndexInCurrentRow, 0, activeFrameCount_ - 1);
    accumulator_ = 0.f;
    applyTextureRect();
}

void Animation::reset()
{
    currentFrame_ = 0;
    accumulator_ = 0.f;
    applyTextureRect();
}

void Animation::play()
{
    playing_ = true;
}

void Animation::pause()
{
    playing_ = false;
}

void Animation::applyTextureRect()
{
    const int column = clampedColumn(startColumn_ + currentFrame_);
    const int row = clampedRow(currentRow_);

    sprite_.setTextureRect(
        sf::IntRect({column * frameSize_.x, row * frameSize_.y}, frameSize_));
}

int Animation::clampedRow(int row) const
{
    return std::clamp(row, 0, rows_ - 1);
}

int Animation::clampedColumn(int column) const
{
    return std::clamp(column, 0, columns_ - 1);
}
} // namespace engine
