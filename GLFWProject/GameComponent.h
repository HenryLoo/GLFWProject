#pragma once
#ifndef GameComponent_H
#define GameComponent_H

#include "AttackPattern.h"
#include "SpriteAnimation.h"
#include "StateMachine.h"

#include <set>
#include <string>
#include <vector>
#include <unordered_map>

class SpriteSheet;

namespace GameComponent
{
	// Defines the different component bitmasks, in powers of 2.
	enum ComponentType
	{
		COMPONENT_NONE = 0,
		COMPONENT_PHYSICS = 1,
		COMPONENT_SPRITE = 2,
		COMPONENT_PLAYER = 4,
		COMPONENT_COLLISION = 8,
		COMPONENT_WEAPON = 16,
		COMPONENT_ATTACK = 32,
		COMPONENT_ENEMY = 64,
		COMPONENT_CHARACTER = 128,
	};

	struct Physics
	{
		// This entity's position.
		glm::vec3 pos;

		// This entity's movement speed.
		glm::vec3 speed;

		// This entity's physical attributes.
		float rotation, weight;
		glm::vec2 scale;

		// Flag for if the entity is being affected by friction.
		bool hasFriction{ true };

		// Flag for if the entity is being affected by gravity.
		bool hasGravity{ true };

		// Flag for if the entity's horizontal direction is locked.
		bool isLockedDirection{ false };
	};

	struct Sprite
	{
		// This sprite's texture.
		SpriteSheet *spriteSheet;

		// This sprite's colour.
		unsigned char r{ 255 }, g{ 255 }, b{ 255 }, a{ 255 };

		// Flag for whether the sprite should be alive after playing its
		// animation.
		// If false, then the entity is deleted after the last frame.
		bool isPersistent{ true };

		// Flag for if the sprite is dead.
		// If true, then the sprite will fade out. Once completely
		// faded, the entity is deleted.
		bool isDead{ false };

		// Hold all frames for this sprite.
		SpriteAnimation currentAnimation;
		int currentFrame{ 0 };
		float currentFrameTime{ 0.f };

		// Distance from the sprite to the camera.
		float cameraDistance{ -1 };

		// Flag for if the animation should be reset.
		bool isResetAnimation{ false };

		bool operator<(const Sprite &that) const
		{
			// Sort in reverse order, so that far particles are drawn first.
			return this->cameraDistance > that.cameraDistance;
		}
	};

	struct Player
	{
		// The maximum number of jumps that the player can perform.
		int numMaxJumps{ 1 };

		// The number of jumps that the player can still perform.
		// This value is reset to numMaxJumps when landing on the ground.
		int numRemainingJumps{ 0 };

		// The maximum number of evades that the player can perform.
		int numMaxEvades{ 1 };

		// The number of evades that the player can still perform.
		// This value is reset to numMaxEvades when landing on the ground.
		int numRemainingEvades{ 0 };

		// The remaining duration in seconds allowing for illusion jumps.
		// An illusion jump refers to jumping during the short period of time
		// after walking off a ledge. This allows for more forgiving jumps.
		float illusionJumpTimer{ 0.f };

		// The remaining duration in seconds for the player remaining in
		// the evade state.
		float evadeTimer{ 0.f };
		float evadeDuration{ 0.f };
	};

	struct Collision
	{
		// The entity's collision box.
		AABB aabb;

		// Flag for if the box has collided from the bottom during this frame.
		bool isCollidingHorizontal{ false };
		bool isCollidingFloor{ false };
		bool isCollidingGhost{ false };
		bool isCollidingSlope{ false };
		bool wasOnGround{ false };

		bool isColliding() const
		{
			return (isCollidingFloor || isCollidingGhost || isCollidingSlope);
		}
	};

	struct Weapon
	{
		// This weapon's texture.
		SpriteSheet *spriteSheet;

		// Hold all frames for this sprite.
		// The weapon's frame timer will be dependent on the entity's sprite
		// component.
		SpriteAnimation currentAnimation;
		int currentFrame{ 0 };

		// Distance from the sprite to the camera.
		float cameraDistance{ -1 };

		// Flag for if the weapon should be shown.
		bool isVisible{ false };

		bool operator<(const Sprite &that) const
		{
			// Sort in reverse order, so that far particles are drawn first.
			return this->cameraDistance > that.cameraDistance;
		}
	};

	struct Attack
	{
		// Flag for if the attack is enabled.
		bool isEnabled{ false };

		// The entity id of source of the attack.
		int sourceId;

		// This attack pattern's values.
		AttackPattern pattern;

		// Hold all ids of entities that have already been hit by this attack.
		std::set<int> hitEntities;
	};

	struct Enemy
	{

	};

	struct Character
	{
		// Hold the character state's label.
		// This label is used as a key for the spritesheet's animations.
		std::string previousState;
		StateMachine states;

		// Map all player states to their appropriate attack patterns.
		std::unordered_map<std::string, AttackPattern> attackPatterns;

		// The remaining duration of time in seconds for the character to remain
		// in a hurt state.
		float hitStunTimer{ 0.f };

		// The remaining duration of time in seconds for the character to remain
		// in a fallen state.
		float fallenTimer{ 0.f };

		// The character's horizontal speed in pixels per second.
		float movementSpeed{ 128.f };

		// The character's vertical speed in pixels per second.
		float jumpSpeed{ 256.f };

		// Character stats.
		int health, resource, power, agility, endurance, focus;
	};

	// Check if an entity has a component.
	bool hasComponent(unsigned long entityCompMask, ComponentType comp);

	// Add a component to an entity.
	void addComponent(unsigned long &entityCompMask, ComponentType comp);

	// Get the duration of the current sprite frame.
	float getFrameDuration(const Sprite &sprite);
}

#endif