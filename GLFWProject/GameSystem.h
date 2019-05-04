#pragma once
#ifndef GameSystem_H
#define GameSystem_H

#include "GameComponent.h"

class SpriteRenderer;

namespace GameSystem
{
	// Update a physics component's values
	void updatePhysics(float deltaTime, GameComponent::Physics &physics);

	// Update a sprite component's values.
	void updateSprite(float deltaTime, SpriteRenderer *renderer, glm::vec3 cameraPos,
		unsigned long &entityCompMask, GameComponent::Sprite &sprite, 
		GameComponent::Physics &physics);
}

#endif