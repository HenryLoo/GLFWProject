#pragma once
#ifndef GameSystem_H
#define GameSystem_H

#include "GameComponent.h"
#include "GameEngine.h"

class SpriteRenderer;
class UIRenderer;
class InputManager;
class Room;

namespace GameSystem
{
	// Functions should return true if the entity is still alive.
	// Otherwise, return false.

	// Update a physics component's values
	void updatePhysics(float deltaTime, int numEntities,
		unsigned long (&entities)[GameEngine::MAX_ENTITIES],
		GameComponent::Physics(&physics)[GameEngine::MAX_ENTITIES]);

	// Update a sprite component's values.
	void updateSprite(float deltaTime, int numEntities,
		unsigned long(&entities)[GameEngine::MAX_ENTITIES],
		SpriteRenderer *renderer, glm::vec3 cameraPos, 
		GameComponent::Sprite(&sprites)[GameEngine::MAX_ENTITIES],
		GameComponent::Physics(&physics)[GameEngine::MAX_ENTITIES]);

	// Update a player component's values
	void updatePlayer(float deltaTime, unsigned long(&playerEntity), InputManager *input, 
		GameComponent::Player &player,
		GameComponent::Physics &physics, GameComponent::Sprite &sprite, 
		GameComponent::Weapon &weapon, GameComponent::Collision &col, 
		GameComponent::Attack &attack);

	// Update collisions using axis-aligned bounding boxes.
	void updateRoomCollision(float deltaTime, int numEntities,
		unsigned long(&entities)[GameEngine::MAX_ENTITIES],
		GameComponent::Physics(&physics)[GameEngine::MAX_ENTITIES],
		GameComponent::Collision(&collisions)[GameEngine::MAX_ENTITIES], 
		Room *room);

	// Update a weapon component based on its entity's sprite component.
	void updateWeapon(float deltaTime, int numEntities,
		unsigned long(&entities)[GameEngine::MAX_ENTITIES],
		SpriteRenderer *renderer, glm::vec3 cameraPos,
		GameComponent::Sprite(&sprites)[GameEngine::MAX_ENTITIES],
		GameComponent::Physics(&physics)[GameEngine::MAX_ENTITIES],
		GameComponent::Weapon(&weapons)[GameEngine::MAX_ENTITIES]);

	// Update an attack component's values and check for collisions against
	// target entities.
	bool updateAttack(float deltaTIme, GameComponent::Sprite &sprite, 
		GameComponent::Attack &attack, GameComponent::Physics &physics,
		int playerId, const std::vector<int> &enemyIds,
		GameComponent::Physics(&targetPhysics)[GameEngine::MAX_ENTITIES],
		GameComponent::Collision(&targetCols)[GameEngine::MAX_ENTITIES]);

	void updateDebug(int numEntities,
		unsigned long(&entities)[GameEngine::MAX_ENTITIES],
		GameComponent::Physics(&physics)[GameEngine::MAX_ENTITIES],
		GameComponent::Collision(&collisions)[GameEngine::MAX_ENTITIES],
		GameComponent::Attack(&attacks)[GameEngine::MAX_ENTITIES],
		UIRenderer *uRenderer);
}

#endif