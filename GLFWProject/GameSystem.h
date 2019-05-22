#pragma once
#ifndef GameSystem_H
#define GameSystem_H

#include "GameComponent.h"

class SpriteRenderer;
class InputManager;
class Room;

namespace GameSystem
{
	// Functions should return true if the entity is still alive.
	// Otherwise, return false.

	// Update a physics component's values
	bool updatePhysics(float deltaTime, GameComponent::Physics &physics);

	// Update a sprite component's values.
	bool updateSprite(float deltaTime, SpriteRenderer *renderer, glm::vec3 cameraPos,
		GameComponent::Sprite &sprite, GameComponent::Physics &physics);

	// Update a player component's values
	bool updatePlayer(InputManager *input, GameComponent::Player &player, 
		GameComponent::Physics &physics, GameComponent::Sprite &sprite);

	// Update collisions using axis-aligned bounding boxes.
	bool updateRoomCollision(float deltaTime, GameComponent::Physics &physics,
		GameComponent::AABB &aabb, Room *room);
}

#endif