#pragma once
#ifndef AttackPattern_H
#define AttackPattern_H

#include <glm/glm.hpp>

struct AttackPattern
{
	// The half value of width and height of the attack's hit box.
	glm::vec2 halfSize;

	// The pixel offset from the box's origin position.
	glm::vec2 offset;

	// The start and end frame index bounds for which this attack is enabled.
	glm::ivec2 frameRange;

	// The amount of damage this attack will deal.
	int damage;
};

#endif