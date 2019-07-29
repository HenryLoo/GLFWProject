#pragma once
#ifndef PlayerSystem_H
#define PlayerSystem_H

#include "GameSystem.h"

class InputManager;

// Update the player entity's values.
class PlayerSystem : public GameSystem
{
public:
	PlayerSystem(GameEngine &game,
		GameComponent::Player &player,
		std::vector<GameComponent::Physics> &physics,
		std::vector<GameComponent::Collision> &collisions);

	virtual void update(float deltaTIme, int numEntities,
		std::vector<unsigned long> &entities);

private:
	virtual void process(float deltaTime, int entityId,
		unsigned long &entityMask);

	// References to the relevant components.
	GameComponent::Player &m_player;
	std::vector<GameComponent::Physics> &m_physics;
	std::vector<GameComponent::Collision> &m_collisions;
};

#endif