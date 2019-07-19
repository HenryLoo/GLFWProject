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

#endif