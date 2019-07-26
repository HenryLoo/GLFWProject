#pragma once
#ifndef CharacterSystem_H
#define CharacterSystem_H

#include "GameSystem.h"

// Update the generic character values, such as character states.
// Weapon and attack components are optional.
class CharacterSystem : public GameSystem
{
public:
	CharacterSystem(GameEngine &game,
		std::vector<GameComponent::Sprite> &sprites,
		std::vector<GameComponent::Weapon> &weapons,
		std::vector<GameComponent::Collision> &collisions,
		std::vector<GameComponent::Attack> &attacks,
		std::vector<GameComponent::Character> &characters);

private:
	virtual void process(float deltaTime, int entityId,
		unsigned long &entityMask);

	// References to the relevant components.
	std::vector<GameComponent::Sprite> &m_sprites;
	std::vector<GameComponent::Weapon> &m_weapons;
	std::vector<GameComponent::Collision> &m_collisions;
	std::vector<GameComponent::Attack> &m_attacks;
	std::vector<GameComponent::Character> &m_characters;
};

#endif