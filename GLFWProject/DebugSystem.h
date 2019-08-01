#pragma once
#ifndef DebugSystem_H
#define DebugSystem_H

#include "GameSystem.h"

// Handle drawing all weapon sprites.
class DebugSystem : public GameSystem
{
public:
	DebugSystem(EntityManager &manager,
		std::vector<GameComponent::Physics> &physics,
		std::vector<GameComponent::Collision> &collisions,
		std::vector<GameComponent::Attack> &attacks);

private:
	virtual void process(float deltaTime, int entityId,
		unsigned long &entityMask);

	// References to the relevant components.
	std::vector<GameComponent::Physics> &m_physics;
	std::vector<GameComponent::Collision> &m_collisions;
	std::vector<GameComponent::Attack> &m_attacks;
};

#endif