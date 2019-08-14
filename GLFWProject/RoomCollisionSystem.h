#pragma once
#ifndef RoomCollisionSystem_H
#define RoomCollisionSystem_H

#include "GameSystem.h"

class GameEngine;

// Handle collisions against the room's walls for all entities.
// Character component is optional.
class RoomCollisionSystem : public GameSystem
{
public:
	RoomCollisionSystem(EntityManager &manager,
		std::vector<GameComponent::Physics> &physics,
		std::vector<GameComponent::Collision> &collisions,
		std::vector<GameComponent::Character> &characters);

private:
	virtual void process(float deltaTime, int entityId,
		unsigned long &entityMask);

	// References to the relevant components.
	std::vector<GameComponent::Physics> &m_physics;
	std::vector<GameComponent::Collision> &m_collisions;
	std::vector<GameComponent::Character> &m_characters;
};

#endif