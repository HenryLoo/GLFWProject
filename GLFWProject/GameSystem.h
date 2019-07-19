#pragma once
#ifndef GameSystem_H
#define GameSystem_H

#include "GameComponent.h"
#include "GameEngine.h"

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
	bool updatePlayer(float deltaTime, InputManager *input, GameComponent::Player &player, 
		GameComponent::Physics &physics, GameComponent::Sprite &sprite, 
		GameComponent::Weapon &weapon, GameComponent::Collision &col, 
		GameComponent::Attack &attack);

	// Update collisions using axis-aligned bounding boxes.
	bool updateRoomCollision(float deltaTime, GameComponent::Physics &physics,
		GameComponent::Collision &col, Room *room);

	// Update a weapon component based on its entity's sprite component.
	bool updateWeapon(float deltaTime, SpriteRenderer *renderer, glm::vec3 cameraPos,
		GameComponent::Sprite &sprite, GameComponent::Physics &physics,
		GameComponent::Weapon &weapon);

	// Update an attack component's values and check for collisions against
	// target entities.
	bool updateAttack(float deltaTIme, GameComponent::Sprite &sprite, 
		GameComponent::Attack &attack, GameComponent::Physics &physics,
		int playerId, const std::vector<int> &enemyIds,
		GameComponent::Physics(&targetPhysics)[GameEngine::MAX_ENTITIES],
		GameComponent::Collision(&targetCols)[GameEngine::MAX_ENTITIES]);
}

#endif