#pragma once
#ifndef PlayerSystem_H
#define PlayerSystem_H

#include "GameSystem.h"

class InputManager;

// Update the player entity's values by handling input.
class PlayerSystem : public GameSystem
{
public:
	PlayerSystem(GameEngine &game,
		GameComponent::Player &player,
		std::vector<GameComponent::Physics> &physics,
		std::vector<GameComponent::Sprite> &sprites,
		std::vector<GameComponent::Weapon> &weapons,
		std::vector<GameComponent::Collision> &collisions,
		std::vector<GameComponent::Attack> &attacks);

	virtual void update(float deltaTIme, int numEntities,
		std::vector<unsigned long> &entities);

private:
	virtual void process(float deltaTime, int entityId,
		unsigned long &entityMask);

	// References to the relevant components.
	GameComponent::Player &m_player;
	std::vector<GameComponent::Physics> &m_physics;
	std::vector<GameComponent::Sprite> &m_sprites;
	std::vector<GameComponent::Weapon> &m_weapons;
	std::vector<GameComponent::Collision> &m_collisions;
	std::vector<GameComponent::Attack> &m_attacks;
};

#endif