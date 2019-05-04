#pragma once
#ifndef GameComponent_H
#define GameComponent_H

#include <glm/glm.hpp>

class Texture;

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
		Texture *texture;

		// This sprite's colour.
		unsigned char r, g, b, a;

		// Determines how long this sprite has left to live, in seconds.
		// If the value is < 0, then this sprite is dead.
		float duration{ -1 };

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