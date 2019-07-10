#pragma once
#ifndef SpriteSheet_H
#define SpriteSheet_H

#include "Texture.h"

#include <unordered_map>
#include <vector>

namespace GameComponent
{
	struct Sprite;
}

struct SpriteAnimation
{
	// The index of this animation's first frame on the sprite sheet.
	int sheetIndex;

	// The number of sprites in this animation.
	int numSprites;

	// Flag for if the sprite animation is looping.
	bool isLooping;

	// The x, y-distances to offset the sprite.
	glm::vec2 offset{ 0.f };

	// The durations of each frame in the animation, in seconds.
	// If there are more frames than durations, take the last duration 
	// value in the vector for that frame.
	std::vector<float> durations;
};

class SpriteSheet : public Texture
{
public:
	SpriteSheet(const std::string &filePath, 
		const std::unordered_map<std::string, SpriteAnimation> &animations,
		glm::ivec2 clipSize);
	SpriteSheet(const std::string &filePath, glm::ivec2 clipSize);
	~SpriteSheet();

	// Get the clip size for this sprite sheet.
	glm::ivec2 getClipSize() const;

	// Find animation corresponding to the given label and output it to the
	// given SpriteComponent reference.
	// Return true if animation was found, else return false.
	bool setAnimation(const std::string &label, GameComponent::Sprite &output) const;

	// Get the sprite sheet's file path.
	const std::string &getFilePath() const;

private:
	// The sprite sheet's file path.
	std::string m_filePath;

	// The width and height of a sprite on the sheet.
	glm::ivec2 m_clipSize;

	// Hold all animations on this sprite sheet.
	// The key refers to the animation's label.
	std::unordered_map<std::string, SpriteAnimation> m_animations;
};

#endif