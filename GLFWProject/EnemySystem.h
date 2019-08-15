#pragma once
#ifndef EnemySystem_H
#define EnemySystem_H

#include "GameSystem.h"

// Update the enemy values.
class EnemySystem : public GameSystem
{
public:
	EnemySystem(EntityManager &manager,
		std::vector<GameComponent::Enemy> &enemies);

private:
	virtual void process(float deltaTime, int entityId,
		unsigned long &entityMask);

	// References to the relevant components.
	std::vector<GameComponent::Enemy> &m_enemies;
};

#endif