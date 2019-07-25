#pragma once
#ifndef RoomCollisionSystem_H
#define RoomCollisionSystem_H

#include "GameSystem.h"

class Room;

// Handle collisions against the room's walls for all entities.
class RoomCollisionSystem : public GameSystem
{
public:
	RoomCollisionSystem(GameEngine &game, 
		std::vector<GameComponent::Physics> &physics,
		std::vector<GameComponent::Collision> &collisions);

private:
	virtual void process(float deltaTime, int entityId,
		unsigned long &entityMask);

	// References to the relevant components.
	std::vector<GameComponent::Physics> &m_physics;
	std::vector<GameComponent::Collision> &m_collisions;
};

#endif