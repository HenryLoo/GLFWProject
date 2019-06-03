#pragma once
#ifndef GameComponent_H
#define GameComponent_H

#include "SpriteSheet.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace GameComponent
{
	// Defines the different component bitmasks, in powers of 2.
	enum ComponentType
	{
		COMPONENT_NONE = 0,
		COMPONENT_PHYSICS = 1,
		COMPONENT_SPRITE = 2,
		COMPONENT_PLAYER = 4,
		COMPONENT_AABB = 8,
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
	};

	struct Sprite
	{
		// This sprite's texture.
		SpriteSheet *spriteSheet;

		// This sprite's colour.
		unsigned char r, g, b, a;

		// Determines how long this sprite has left to live, in seconds.
		// If the value is < 0, then this sprite is dead.
		float duration{ -1 };
		bool hasDuration{ true };

		// Hold all frames for this sprite.
		//std::vector<SpriteFrame> frames{ SpriteFrame{} };
		SpriteAnimation currentAnimation;
		int currentFrame{ 0 };
		float currentFrameTime{ 0.f };

		// Distance from the sprite to the camera.
		float cameraDistance{ -1 };

		bool operator<(const Sprite &that) const
		{
			// Sort in reverse order : far particles drawn first.
			return this->cameraDistance > that.cameraDistance;
		}
	};

	struct Player
	{
		// Hold the current state's label.
		// This label is used as a key for the spritesheet's animations.
		std::string currentState;

		// The maximum number of jumps that the player can perform.
		int numMaxJumps{ 1 };

		// The number of jumps that the player can still perform.
		// This value is reset to numMaxJumps when landing on the ground.
		int numRemainingJumps{ 0 };
	};

	struct AABB
	{
		// The half value of width and height.
		glm::vec2 halfSize;

		// The pixel offset from the box's origin position.
		glm::vec2 offset;

		// Flag for if the box has collided from the bottom during this frame.
		bool isCollidingTop{ false };
		bool isCollidingBottom{ false };
	};

	// Check if an entity has a component.
	bool hasComponent(unsigned long entityCompMask, ComponentType comp);

	// Add a component to an entity.
	void addComponent(unsigned long &entityCompMask, ComponentType comp);

	// Get the duration of the current sprite frame.
	float getFrameDuration(const Sprite &sprite);
}

#endif