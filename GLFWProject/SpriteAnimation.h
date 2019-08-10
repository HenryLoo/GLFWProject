#pragma once
#ifndef SpriteAnimation_H
#define SpriteAnimation_H

#include <glm/glm.hpp>

#include <vector>

struct SpriteAnimation
{
	// The index of this animation's first frame on the sprite sheet.
	int firstIndex{ 0 };

	// The number of sprites in this animation.
	int numSprites{ 1 };

	// Flag for if the sprite animation is looping.
	bool isLooping{ false };

	// The x, y-distances to offset the sprite.
	glm::vec2 offset{ 0.f };

	// The durations of each frame in the animation, in seconds.
	// If there are more frames than durations, take the last duration 
	// value in the vector for that frame.
	std::vector<float> durations;
};

#endif;