#pragma once
#ifndef AABB_H
#define AABB_H

#include <glm/glm.hpp>

struct AABB
{
	// The half value of width and height of the box.
	glm::vec2 halfSize;

	// The pixel offset from the box's origin position.
	glm::vec2 offset;
};

// An AABB augmented with details about its source.
struct AABBSource
{
	enum class Type
	{
		Collision,
		Attack
	};

	// The id of the entity that this endpoint belongs to.
	int entityId;

	// The type of the endpoint: collision box or attack hit box.
	Type type;
};

#endif