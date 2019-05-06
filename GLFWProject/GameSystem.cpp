#include "GameSystem.h"
#include "SpriteRenderer.h"

#include <glm/gtx/norm.hpp>

#include <iostream>

void GameSystem::updatePhysics(float deltaTime, GameComponent::Physics &physics)
{
	// Update this component's values.
	physics.speed += glm::vec3(0.0f, -3.f, 0.0f);
	physics.speed.y = glm::max(-3.f, physics.speed.y);
	physics.pos += physics.speed * deltaTime;
	physics.pos.y = glm::max(0.f, physics.pos.y);
}

void GameSystem::updateSprite(float deltaTime, SpriteRenderer *renderer, 
	glm::vec3 cameraPos, unsigned long &entityCompMask, 
	GameComponent::Sprite &sprite, GameComponent::Physics &physics)
{
	// Do nothing if the sprite is time-based and the duration is over.
	if (sprite.duration <= 0.f && sprite.hasDuration)
	{
		return;
	}

	// Decrease the sprite's duration if the sprite is time-limited.
	if (sprite.hasDuration)
		sprite.duration -= deltaTime;

	// Update sprite values for this frame.
	if (sprite.duration > 0.0f || !sprite.hasDuration)
	{
		// Update this sprite's values.
		sprite.cameraDistance = glm::length2(physics.pos - cameraPos);

		// Update this sprite's animation.
		sprite.currentFrameTime += deltaTime;
		float frameDuration{ sprite.frames[sprite.currentFrame].duration };

		// Process the next frame if the current frame is over and the 
		// animation is not a non-looping one at its last frame.
		if (sprite.currentFrameTime >= frameDuration &&
			!(!sprite.isLooping && sprite.currentFrame == sprite.frames.size() - 1 ))
		{
			// Check if deltaTime has accumulated a value greater than the
			// frame's duration, and then find how many frames should be
			// processed.
			float leftoverTime = sprite.currentFrameTime - frameDuration;
			do
			{
				sprite.currentFrameTime = leftoverTime;
				sprite.currentFrame++;
				if (sprite.currentFrame >= sprite.frames.size())
					sprite.currentFrame = 0;

				// Get how long this new frame lasts and see if there is still
				// enough leftover time to process it.
				frameDuration = sprite.frames[sprite.currentFrame].duration;
				leftoverTime -= frameDuration;
			} while (leftoverTime >= frameDuration);
		}
	}
	// The sprite is time-limited and this is its last frame.
	else
	{
		// Set distance to camera to be minimum value so that
		// it will be placed at the back of the sprites array when sorted.
		sprite.cameraDistance = -1.0f;

		// Remove all components from the entity.
		entityCompMask = 0;
	}

	// Update the renderer's array of sprites.
	renderer->updateSprites(physics, sprite);
	renderer->incrementNumSprites();
}