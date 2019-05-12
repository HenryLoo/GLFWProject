#pragma once
#ifndef GameComponent_H
#define GameComponent_H

#include "SpriteSheet.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

//struct SpriteFrame
//{
//	// The index of the sprite on the texture atlas.
//	int spriteIndex{ 0 };
//
//	// How long this frame lasts, in seconds.
//	// Set to -1 if the frame lasts indefinitely.
//	float duration{ -1.f };
//};

namespace GameComponent
{
	// Defines the different component bitmasks, in powers of 2.
	enum ComponentType
	{
		COMPONENT_NONE = 0,
		COMPONENT_PHYSICS = 1,
		COMPONENT_SPRITE = 2
	};

	// Check if an entity has a component.
	bool hasComponent(unsigned long entityCompMask, ComponentType comp);

	// Add a component to an entity.
	void addComponent(unsigned long &entityCompMask, ComponentType comp);

	struct Physics
	{
		// This sprite's position.
		glm::vec3 pos;

		// This sprite's movement speed.
		glm::vec3 speed;

		// This sprite's physical attributes.
		float scale, angle, weight;
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

		// Flag for if the sprite animation is looping.
		bool isLooping{ false };

		// Distance from the sprite to the camera.
		float cameraDistance{ -1 };

		bool operator<(const Sprite &that) const
		{
			// Sort in reverse order : far particles drawn first.
			return this->cameraDistance > that.cameraDistance;
		}
	};
}

#endif