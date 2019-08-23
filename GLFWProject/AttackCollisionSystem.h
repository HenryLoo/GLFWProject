#pragma once
#ifndef AttackCollisionSystem_H
#define AttackCollisionSystem_H

#include "GameSystem.h"

namespace SoLoud
{
	class Soloud;
}

// Handles collisions involving entities and attacks.
class AttackCollisionSystem : public GameSystem
{
public:
	AttackCollisionSystem(EntityManager &manager,
		SoLoud::Soloud &soundEngine,
		std::vector<GameComponent::Physics> &physics,
		std::vector<GameComponent::Sprite> &sprites,
		std::vector<GameComponent::Collision> &collisions,
		std::vector<GameComponent::Attack> &attacks,
		std::vector<GameComponent::Character> &characters,
		GameComponent::Player &player);

	virtual void update(float deltaTime, int numEntities,
		std::vector<unsigned long> &entities);

private:
	virtual void process(float deltaTime, int entityId,
		unsigned long &entityMask);

	// Reference to the sound engine.
	SoLoud::Soloud &m_soundEngine;

	// References to the relevant components.
	std::vector<GameComponent::Physics> &m_physics;
	std::vector<GameComponent::Sprite> &m_sprites;
	std::vector<GameComponent::Collision> &m_collisions;
	std::vector<GameComponent::Attack> &m_attacks;
	std::vector<GameComponent::Character> &m_characters;
	GameComponent::Player &m_player;
};

#endif