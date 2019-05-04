#include "GameSystem.h"
#include "SpriteRenderer.h"

#include <glm/gtx/norm.hpp>

void GameSystem::updatePhysics(float deltaTime, GameComponent::Physics &physics)
{
	// Update this component's values.
	physics.speed += glm::vec3(0.0f, -9.81f, 0.0f) * deltaTime * 0.5f;
	physics.pos += physics.speed * deltaTime;
}

void GameSystem::updateSprite(float deltaTime, SpriteRenderer *renderer, 
	glm::vec3 cameraPos, unsigned long &entityCompMask, 
	GameComponent::Sprite &sprite, GameComponent::Physics &physics)
{
	// Update sprite values for this frame.
	if (sprite.duration > 0.0f)
	{
		sprite.duration -= deltaTime;
		if (sprite.duration > 0.0f)
		{
			// Update this sprite's values.
			sprite.cameraDistance = glm::length2(physics.pos - cameraPos);

			// Update the renderer's array of sprites.
			renderer->updateSprites(physics, sprite);
		}
		else
		{
			// Set distance to camera to be minimum value so that
			// it will be placed at the back of the sprites array when sorted.
			sprite.cameraDistance = -1.0f;

			// Remove all components from the entity.
			entityCompMask = 0;
		}

		renderer->incrementNumSprites();
	}
}