#pragma once
#ifndef GameSystem_H
#define GameSystem_H

#include "GameComponent.h"

class SpriteRenderer;

namespace GameSystem
{
	// Functions should return true if the entity is still alive.
	// Otherwise, return false.

	// Update a physics component's values
	bool updatePhysics(float deltaTime, GameComponent::Physics &physics);

	// Update a sprite component's values.
	bool updateSprite(float deltaTime, SpriteRenderer *renderer, glm::vec3 cameraPos,
		GameComponent::Sprite &sprite, GameComponent::Physics &physics);
}

#endif