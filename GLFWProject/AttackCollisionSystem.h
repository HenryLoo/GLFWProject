#pragma once
#ifndef AttackCollisionSystem_H
#define AttackCollisionSystem_H

#include "GameSystem.h"

// Handles collisions involving entities and attacks.
class AttackCollisionSystem : public GameSystem
{
public:
	AttackCollisionSystem(GameEngine &game,
		std::vector<GameComponent::Physics> &physics,
		std::vector<GameComponent::Sprite> &sprites,
		std::vector<GameComponent::Collision> &collisions,
		std::vector<GameComponent::Attack> &attacks,
		std::vector<GameComponent::Character> &characters);

	virtual void update(float deltaTime, int numEntities,
		std::vector<unsigned long> &entities);

private:
	virtual void process(float deltaTime, int entityId,
		unsigned long &entityMask);

	// References to the relevant components.
	std::vector<GameComponent::Physics> &m_physics;
	std::vector<GameComponent::Sprite> &m_sprites;
	std::vector<GameComponent::Collision> &m_collisions;
	std::vector<GameComponent::Attack> &m_attacks;
	std::vector<GameComponent::Character> &m_characters;
};

#endif