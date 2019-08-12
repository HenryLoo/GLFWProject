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

class SpriteSheet : public Texture
{
public:
	struct SpriteClip
	{
		// The width and height of this clip in pixels.
		glm::ivec2 clipSize{ 0 };

		// The top-left pixel coordinate of this clip on the sprite sheet.
		glm::ivec2 topLeft{ 0 };

		// The x, y-distances to offset the sprite.
		glm::vec2 offset{ 0.f };

		// The durations of this clip frame, if animated, in seconds.
		float duration{ -1.f };
	};

	struct SpriteSet
	{
		// Flag for if the sprite animation is looping.
		bool isLooping{ false };

		// If there is more than one clip in the set, then it is animated.
		std::vector<SpriteClip> clips;
	};

	SpriteSheet(GLuint id, GLint width, GLint height, GLint numChannels,
		const std::unordered_map<std::string, SpriteSet> &sprites, 
		std::string name);
	SpriteSheet(GLuint id, GLint width, GLint height, GLint numChannels, 
		std::string name);

	// Find the SpriteSet corresponding to the given label and initialze it in the
	// given SpriteComponent reference.
	// Return true if animation was found, else return false.
	bool setSprite(const std::string &label, GameComponent::Sprite &output) const;

	// Find the SpriteSet corresponding to the given label and output it to the
	// given SpriteSet reference.
	// Return true if animation was found, else return false.
	bool getSprite(const std::string &label, SpriteSet &sprite) const;

	// Get the sprite sheet's name.
	const std::string &getName() const;

private:
	// The sprite sheet's name.
	std::string m_name;

	// Hold all sprites on this sprite sheet.
	// The key refers to each sprite's label.
	std::unordered_map<std::string, SpriteSet> m_sprites;
};

#endif