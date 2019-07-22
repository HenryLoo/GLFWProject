#pragma once
#ifndef GameSystem_H
#define GameSystem_H

#include "GameComponent.h"

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
		std::vector<unsigned long> &entities,
		std::vector<GameComponent::Physics> &physics);

	// Update a sprite component's values.
	void updateSprite(float deltaTime, int numEntities,
		std::vector<unsigned long> &entities,
		SpriteRenderer *renderer, glm::vec3 cameraPos,
		std::vector<GameComponent::Sprite> &sprites,
		std::vector<GameComponent::Physics> &physics);

	// Update a player component's values
	void updatePlayer(float deltaTime, unsigned long(&playerEntity), InputManager *input, 
		GameComponent::Player &player,
		GameComponent::Physics &physics, GameComponent::Sprite &sprite, 
		GameComponent::Weapon &weapon, GameComponent::Collision &col, 
		GameComponent::Attack &attack);

	// Update collisions using axis-aligned bounding boxes.
	void updateRoomCollision(float deltaTime, int numEntities,
		std::vector<unsigned long> &entities,
		std::vector<GameComponent::Physics> &physics,
		std::vector<GameComponent::Collision> &collisions,
		Room *room);

	// Update a weapon component based on its entity's sprite component.
	void updateWeapon(float deltaTime, int numEntities,
		std::vector<unsigned long> &entities,
		SpriteRenderer *renderer, glm::vec3 cameraPos,
		std::vector<GameComponent::Sprite> &sprites,
		std::vector<GameComponent::Physics> &physics,
		std::vector<GameComponent::Weapon> &weapons);

	// Update an attack component's values and check for collisions against
	// target entities.
	bool updateAttack(float deltaTIme, GameComponent::Sprite &sprite, 
		GameComponent::Attack &attack, GameComponent::Physics &physics,
		int playerId, const std::vector<int> &enemyIds,
		std::vector<GameComponent::Physics> &targetPhysics,
		std::vector<GameComponent::Collision> &targetCols);

	void updateDebug(int numEntities,
		std::vector<unsigned long> &entities, 
		std::vector<GameComponent::Physics> &physics,
		std::vector<GameComponent::Collision> &collisions,
		std::vector<GameComponent::Attack> &attacks,
		UIRenderer *uRenderer);
}

#endif