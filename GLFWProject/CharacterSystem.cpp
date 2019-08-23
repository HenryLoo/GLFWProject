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

	const unsigned char MAX_ALPHA{ 255 };

	// Constants for sprite blinking when dead.
	const unsigned char DEAD_MAX_ALPHA{ 100 };
	const unsigned char DEAD_MIN_ALPHA{ 20 };
	const float DEAD_ALPHA_SPEED{ 20.f };

	// Constants for sprite blinking when invincible.
	const unsigned char INVINCIBLE_MAX_ALPHA{ 230 };
	const unsigned char INVINCIBLE_MIN_ALPHA{ 170 };
	const float INVINCIBLE_ALPHA_SPEED{ 20.f };

	// Constants for super armour colours;
	const unsigned char SUPER_ARMOUR_G{ 200 };
	const unsigned char SUPER_ARMOUR_B{ 100 };
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
	// Update sprite colour based on character status.
	updateStatusColour(deltaTime, entityId);

	// Update the hit stop timer.
	GameComponent::Character &character{ m_characters[entityId] };
	GameComponent::updateTimer(deltaTime, character.hitStopTimer);

	// If hit stopped, then skip this.
	if (GameComponent::hasHitStop(entityMask, character))
	{
		return;
	}

	// Process only the first frame when hit stop begins.
	character.isFirstHitStopFrame = false;

	GameComponent::Collision &collision{ m_collisions[entityId] };

	character.previousState = character.states.getState();

	// Check the character's state edges.
	character.states.checkEdges(entityId);

	// Change the sprite's state if it is a different one or the animation
	// is being reset.
	updateSprite(entityId, entityMask);

	// Update the character's state machine.
	character.states.update(entityId);

	// Update the character's timers.
	updateTimers(deltaTime, entityId);

	// Delete the entity if it is dead.
	if (GameComponent::isDead(character) && character.fallenTimer == 0.f &&
		character.states.getState() == CharState::FALLEN &&
		entityId != m_manager.getPlayerId())
	{
		m_manager.deleteEntity(entityId);
		return;
	}
}

void CharacterSystem::updateStatusColour(float deltaTime, int entityId)
{
	GameComponent::Character &character{ m_characters[entityId] };
	GameComponent::Sprite &spr{ m_sprites[entityId] };
	GameComponent::Attack &atk{ m_attacks[entityId] };

	unsigned char r, g, b, a;
	r = g = b = a = MAX_ALPHA;

	if (GameComponent::isDead(character) && 
		character.states.getState() == CharState::FALLEN)
	{
		a = static_cast<unsigned char>((DEAD_MAX_ALPHA - DEAD_MIN_ALPHA) *
			glm::abs(glm::sin(character.fallenTimer * DEAD_ALPHA_SPEED)) +
			DEAD_MIN_ALPHA);
	}
	else if (GameComponent::isInvincible(character))
	{
		a = static_cast<unsigned char>((INVINCIBLE_MAX_ALPHA - INVINCIBLE_MIN_ALPHA) *
			glm::abs(glm::sin(character.invincibilityTimer * INVINCIBLE_ALPHA_SPEED)) +
			INVINCIBLE_MIN_ALPHA);
	}
	else if (GameComponent::hasSuperArmour(spr, atk))
	{
		g = SUPER_ARMOUR_G;
		b = SUPER_ARMOUR_B;
	}

	spr.r = r;
	spr.g = g;
	spr.b = b;
	spr.a = a;
}

void CharacterSystem::updateSprite(int entityId, unsigned long &entityMask)
{
	GameComponent::Character &character{ m_characters[entityId] };
	GameComponent::Sprite &spr{ m_sprites[entityId] };
	GameComponent::Attack &atk{ m_attacks[entityId] };

	const std::string &currentState{ character.states.getState() };
	if (currentState != character.previousState || spr.isResetAnimation)
	{
		spr.isResetAnimation = false;
		spr.spriteSheet->setSprite(currentState, spr);

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
				atk.pattern = it->second;
			}
			else
			{
				atk.pattern = {};
			}

			atk.hitEntities.clear();
		}
	}
}

void CharacterSystem::updateTimers(float deltaTime, 
	int entityId)
{
	GameComponent::Character &character{ m_characters[entityId] };

	// Update the hit stun timer.
	GameComponent::updateTimer(deltaTime, character.hitStunTimer);

	// Update the fallen timer.
	GameComponent::updateTimer(deltaTime, character.fallenTimer);

	// Update the invincibility timer.
	GameComponent::updateTimer(deltaTime, character.invincibilityTimer);
}