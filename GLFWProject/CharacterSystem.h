#pragma once
#ifndef CharacterSystem_H
#define CharacterSystem_H

#include "GameSystem.h"

// Update the generic character values, such as character states.
// Weapon and attack components are optional.
class CharacterSystem : public GameSystem
{
public:
	CharacterSystem(EntityManager &manager,
		std::vector<GameComponent::Sprite> &sprites,
		std::vector<GameComponent::Weapon> &weapons,
		std::vector<GameComponent::Collision> &collisions,
		std::vector<GameComponent::Attack> &attacks,
		std::vector<GameComponent::Character> &characters);

private:
	virtual void process(float deltaTime, int entityId,
		unsigned long &entityMask);

	// Update sprite colour based on character status.
	void updateStatusColour(float deltaTime, int entityId);

	// Change the sprite's state if it is a different one or the animation
	// is being reset.
	void updateSprite(int entityId, unsigned long &entityMask);

	// Update the character's timers.
	void updateTimers(float deltaTime, int entityId);

	// References to the relevant components.
	std::vector<GameComponent::Sprite> &m_sprites;
	std::vector<GameComponent::Weapon> &m_weapons;
	std::vector<GameComponent::Collision> &m_collisions;
	std::vector<GameComponent::Attack> &m_attacks;
	std::vector<GameComponent::Character> &m_characters;
};

#endif