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
	SpriteSheet(GLuint id, GLint width, GLint height, GLint numChannels,
		const std::unordered_map<std::string, SpriteAnimation> &animations,
		glm::ivec2 clipSize, std::string name);
	SpriteSheet(GLuint id, GLint width, GLint height, GLint numChannels, 
		glm::ivec2 clipSize, std::string name);

	// Get the clip size for this sprite sheet.
	glm::ivec2 getClipSize() const;

	// Find animation corresponding to the given label and output it to the
	// given SpriteComponent reference.
	// Return true if animation was found, else return false.
	bool setAnimation(const std::string &label, GameComponent::Sprite &output) const;

	// Get the sprite sheet's name.
	const std::string &getName() const;

private:
	// The sprite sheet's name.
	std::string m_name;

	// The width and height of a sprite on the sheet.
	glm::ivec2 m_clipSize;

	// Hold all animations on this sprite sheet.
	// The key refers to the animation's label.
	std::unordered_map<std::string, SpriteAnimation> m_animations;
};

#endif