#include "CharacterSystem.h"

#include "CharStates.h"
#include "SpriteSheet.h"

#include <glm/glm.hpp>

namespace
{
	// The number of seconds to set the fallen timer.
	const float FALLEN_DURATION{ 3.f };
}

CharacterSystem::CharacterSystem(GameEngine &game,
	std::vector<GameComponent::Sprite> &sprites,
	std::vector<GameComponent::Weapon> &weapons,
	std::vector<GameComponent::Collision> &collisions,
	std::vector<GameComponent::Attack> &attacks,
	std::vector<GameComponent::Character> &characters) :
	GameSystem(game, { GameComponent::COMPONENT_SPRITE,
		GameComponent::COMPONENT_CHARACTER }),
	m_sprites(sprites), m_weapons(weapons), m_collisions(collisions),
	m_attacks(attacks), m_characters(characters)
{

}

void CharacterSystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	GameComponent::Sprite &sprite{ m_sprites[entityId] };
	GameComponent::Collision &collision{ m_collisions[entityId] };
	GameComponent::Attack &attack{ m_attacks[entityId] };
	GameComponent::Character &character{ m_characters[entityId] };

	// Handle common sprite transitions.
	// If the current frame is over and the animation is a non-looping one 
	// at its last frame.
	float frameDuration{ GameComponent::getFrameDuration(sprite) };
	if (sprite.currentFrameTime >= frameDuration &&
		(!sprite.currentAnimation.isLooping && sprite.currentFrame == sprite.currentAnimation.numSprites - 1))
	{
		const std::string &currentState{ character.currentState };
		if ((currentState == CharState::HURT && character.hitStunTimer == 0.f) ||
			(currentState == CharState::FALLEN && character.fallenTimer == 0.f))
		{
			character.nextState = CharState::IDLE;
		}
		else if (currentState == CharState::HURT_AIR &&
			(collision.isCollidingFloor || collision.isCollidingGhost || collision.isCollidingSlope))
		{
			character.nextState = CharState::FALLEN;
			character.fallenTimer = FALLEN_DURATION;
		}
	}

	// Change the sprite's state if it is a different one or the animation
	// is being reset.
	if (character.currentState != character.nextState || sprite.isResetAnimation)
	{
		character.currentState = character.nextState;
		sprite.isResetAnimation = false;
		sprite.spriteSheet->setAnimation(character.currentState, sprite);

		// Weapon component is optional.
		if (GameComponent::hasComponent(entityMask, GameComponent::COMPONENT_WEAPON))
		{
			// Create a temporary sprite component to get the weapon's 
			// sprite animation. Set the animation if it exists.
			GameComponent::Weapon &weapon{ m_weapons[entityId] };
			GameComponent::Sprite weaponSprite;
			weapon.isVisible = weapon.spriteSheet->setAnimation(character.currentState, weaponSprite);
			if (weapon.isVisible)
			{
				weapon.currentAnimation = weaponSprite.currentAnimation;
			}
		}

		// Attack component is optional.
		if (GameComponent::hasComponent(entityMask, GameComponent::COMPONENT_ATTACK))
		{
			// Set the attack pattern for this state if it exists.
			auto it{ character.attackPatterns.find(character.currentState) };
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
	if (character.hitStunTimer > 0.f)
	{
		character.hitStunTimer -= deltaTime;
		character.hitStunTimer = glm::max(0.f, character.hitStunTimer);
	}

	// Update the fallen timer.
	if (character.fallenTimer > 0.f)
	{
		character.fallenTimer -= deltaTime;
		character.fallenTimer = glm::max(0.f, character.fallenTimer);
	}
}