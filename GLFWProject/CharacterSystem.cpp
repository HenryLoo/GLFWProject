#include "CharacterSystem.h"

#include "CharStates.h"
#include "SpriteSheet.h"
#include "EntityManager.h"

#include <glm/glm.hpp>
#include <iostream>

namespace
{
	// The number of seconds to set the fallen timer.
	const float FALLEN_DURATION{ 3.f };
}

CharacterSystem::CharacterSystem(EntityManager &manager,
	std::vector<GameComponent::Sprite> &sprites,
	std::vector<GameComponent::Weapon> &weapons,
	std::vector<GameComponent::Collision> &collisions,
	std::vector<GameComponent::Attack> &attacks,
	std::vector<GameComponent::Character> &characters) :
	GameSystem(manager, { GameComponent::COMPONENT_SPRITE,
		GameComponent::COMPONENT_CHARACTER }),
	m_sprites(sprites), m_weapons(weapons), m_collisions(collisions),
	m_attacks(attacks), m_characters(characters)
{

}

void CharacterSystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	GameComponent::Character &character{ m_characters[entityId] };

	// Update the hit stop timer.
	GameComponent::updateTimer(deltaTime, character.hitStopTimer);

	// If hit stopped, then skip this.
	if (GameComponent::hasHitStop(entityMask, character))
	{
		return;
	}

	// Process only the first frame when hit stop begins.
	character.isFirstHitStopFrame = false;

	GameComponent::Sprite &sprite{ m_sprites[entityId] };
	GameComponent::Collision &collision{ m_collisions[entityId] };
	GameComponent::Attack &attack{ m_attacks[entityId] };

	// Update the character's state machine.
	character.states.update(entityId);

	// Change the sprite's state if it is a different one or the animation
	// is being reset.
	const std::string &currentState{ character.states.getState() };
	if (currentState != character.previousState || sprite.isResetAnimation)
	{
		sprite.isResetAnimation = false;
		sprite.spriteSheet->setSprite(currentState, sprite);

		// Weapon component is optional.
		if (GameComponent::hasComponent(entityMask, GameComponent::COMPONENT_WEAPON))
		{
			// Create a temporary sprite component to get the weapon's 
			// sprite animation. Set the animation if it exists.
			GameComponent::Weapon &weapon{ m_weapons[entityId] };
			GameComponent::Sprite weaponSprite;
			weapon.isVisible = weapon.spriteSheet->setSprite(currentState, weaponSprite);
			if (weapon.isVisible)
			{
				weapon.currentSprite = weaponSprite.currentSprite;
			}
		}

		// Attack component is optional.
		if (GameComponent::hasComponent(entityMask, GameComponent::COMPONENT_ATTACK))
		{
			// Set the attack pattern for this state if it exists.
			auto it{ character.attackPatterns.find(currentState) };
			if (it != character.attackPatterns.end())
			{
				attack.pattern = it->second;
			}
			else
			{
				attack.pattern = {};
			}

			attack.hitEntities.clear();
		}
	}

	// Update the hit stun timer.
	GameComponent::updateTimer(deltaTime, character.hitStunTimer);

	// Update the fallen timer.
	GameComponent::updateTimer(deltaTime, character.fallenTimer);

	// Delete the entity if it is dead.
	if (GameComponent::isDead(character) && character.fallenTimer == 0.f &&
		character.states.getState() == CharState::FALLEN &&
		entityId != m_manager.getPlayerId())
	{
		m_manager.deleteEntity(entityId);
		return;
	}

	// Update the previous state, so that it will be ready for the next 
	// iteration.
	character.previousState = currentState;
}