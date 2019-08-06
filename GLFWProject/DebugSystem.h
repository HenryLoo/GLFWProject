#pragma once
#ifndef DebugSystem_H
#define DebugSystem_H

#include "GameSystem.h"

class UIRenderer;

// Handle drawing all weapon sprites.
class DebugSystem : public GameSystem
{
public:
	DebugSystem(EntityManager &manager,
		UIRenderer *uRenderer,
		std::vector<GameComponent::Physics> &physics,
		std::vector<GameComponent::Collision> &collisions,
		std::vector<GameComponent::Attack> &attacks);

private:
	virtual void process(float deltaTime, int entityId,
		unsigned long &entityMask);

	// Pointer to the UI renderer.
	UIRenderer *m_uRenderer{ nullptr };

	// References to the relevant components.
	std::vector<GameComponent::Physics> &m_physics;
	std::vector<GameComponent::Collision> &m_collisions;
	std::vector<GameComponent::Attack> &m_attacks;
};

#endif