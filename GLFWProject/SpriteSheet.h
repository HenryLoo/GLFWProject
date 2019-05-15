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

	// The durations of each frame in the animation, in seconds.
	std::vector<float> durations;
};

class SpriteSheet : public Texture
{
public:
	SpriteSheet(const std::string &filePath, 
		const std::unordered_map<std::string, SpriteAnimation> &animations,
		glm::vec2 clipSize);
	~SpriteSheet();

	// Get the clip size for this sprite sheet.
	glm::vec2 getClipSize() const;

	// Find animation corresponding to the given label and output it to the
	// given SpriteComponent reference.
	void setAnimation(const std::string &label, GameComponent::Sprite &output) const;

private:
	// The width and height of a sprite on the sheet.
	glm::vec2 m_clipSize;

	// Hold all animations on this sprite sheet.
	// The key refers to the animation's label.
	std::unordered_map<std::string, SpriteAnimation> m_animations;
};

#endif