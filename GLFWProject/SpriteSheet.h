#pragma once
#ifndef SpriteSheet_H
#define SpriteSheet_H

#include "Texture.h"
#include "SpriteAnimation.h"

#include <unordered_map>
#include <vector>

namespace GameComponent
{
	struct Sprite;
}

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