#pragma once
#ifndef PhysicsSystem_H
#define PhysicsSystem_H

#include "GameSystem.h"

// Update physics for all entities.
class PhysicsSystem : public GameSystem
{
public:
	PhysicsSystem(EntityManager &manager,
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