#pragma once
#ifndef AttackSystem_H
#define AttackSystem_H

#include "GameSystem.h"

// Update attack values.
class AttackSystem : public GameSystem
{
public:
	AttackSystem(EntityManager &manager,
		std::vector<GameComponent::Sprite> &sprites,
		std::vector<GameComponent::Attack> &attacks);

private:
	virtual void process(float deltaTime, int entityId,
		unsigned long &entityMask);

	// References to the relevant components.
	std::vector<GameComponent::Sprite> &m_sprites;
	std::vector<GameComponent::Attack> &m_attacks;
};

#endif