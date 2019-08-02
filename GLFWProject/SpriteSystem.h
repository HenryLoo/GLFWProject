#pragma once
#ifndef SpriteSystem_H
#define SpriteSystem_H

#include "GameSystem.h"

class SpriteRenderer;
class Camera;

// Handle drawing sprites for all entities.
// Weapon and character components is optional.
class SpriteSystem : public GameSystem
{
public:
	SpriteSystem(EntityManager &manager,
		std::vector<GameComponent::Physics> &physics,
		std::vector<GameComponent::Sprite> &sprites,
		std::vector<GameComponent::Weapon> &weapons,
		std::vector<GameComponent::Character> &characters);

private:
	virtual void process(float deltaTime, int entityId,
		unsigned long &entityMask);

	// References to the relevant components.
	std::vector<GameComponent::Physics> &m_physics;
	std::vector<GameComponent::Sprite> &m_sprites;
	std::vector<GameComponent::Weapon> &m_weapons;
	std::vector<GameComponent::Character> &m_characters;
};

#endif