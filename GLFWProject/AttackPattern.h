#pragma once
#ifndef AttackPattern_H
#define AttackPattern_H

#include "AABB.h"

#include <memory>

class Sound;

struct AttackPattern
{
	// The attack's collision box.
	AABB aabb;

	// The start and end frame index bounds for which this attack is enabled.
	glm::ivec2 frameRange;

	// The minimum frame index to allow for an additional attack.
	int comboFrame;

	// The sound to play when a target is hit.
	std::shared_ptr<Sound> hitSound;

	// The amount of damage this attack will deal.
	int damage;

	// The amount knockback this attack will deal.
	// Knockback will be applied to the target's speed.
	glm::vec2 knockback;
};

#endif