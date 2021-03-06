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
		SpriteRenderer *sRenderer,
		std::vector<GameComponent::Physics> &physics,
		std::vector<GameComponent::Sprite> &sprites,
		std::vector<GameComponent::Weapon> &weapons);

private:
	virtual void process(float deltaTime, int entityId,
		unsigned long &entityMask);

	// Pointer to the sprite renderer.
	SpriteRenderer *m_sRenderer{ nullptr };

	// References to the relevant components.
	std::vector<GameComponent::Physics> &m_physics;
	std::vector<GameComponent::Sprite> &m_sprites;
	std::vector<GameComponent::Weapon> &m_weapons;
};

#endif