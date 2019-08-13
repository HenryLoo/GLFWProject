#pragma once
#ifndef AttackPattern_H
#define AttackPattern_H

#include "AABB.h"

#include <memory>
#include <string>

class Sound;

struct AttackPattern
{
	// The attack's collision box.
	AABB aabb;

	// The start and end frame index bounds for which this attack is enabled.
	glm::ivec2 frameRange{ 0 };

	// The minimum frame index to allow for an additional attack.
	int comboFrame{ -1 };

	// The sound to play when a target is hit.
	std::shared_ptr<Sound> hitSound;

	// The effect to show when a target is hit.
	std::string hitSpark;

	// The duration of cooldown in seconds.
	// This determines how long must be waited before this attack can be 
	// performed again.
	float cooldown{ 0.f };

	// The amount of damage this attack will deal.
	int damage{ 0 };

	// The amount knockback this attack will deal.
	// Knockback will be applied to the target's speed.
	glm::vec2 knockback{ 0.f };
};

#endif