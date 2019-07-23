#pragma once
#ifndef AttackPattern_H
#define AttackPattern_H

#include "AABB.h"

struct AttackPattern
{
	// The attack's collision box.
	AABB aabb;

	// The start and end frame index bounds for which this attack is enabled.
	glm::ivec2 frameRange;

	// The amount of damage this attack will deal.
	int damage;

	// The amount knockback this attack will deal.
	// Knockback will be applied to the target's speed.
	glm::vec2 knockback;
};

#endif