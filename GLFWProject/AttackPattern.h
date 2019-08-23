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

	// The frame index range for which this attack is enabled.
	int hitStart{ -1 };
	int hitFrames{ 0 };

	// The start and end frame index bounds for which this attack provides
	// super armour. Super armour prevents the attacker from being put
	// in a hurt state or launched when taking damage.
	int superArmourStart{ -1 };
	int superArmourFrames{ 0 };

	// The minimum frame index to allow for an additional attack.
	int comboFrame{ -1 };

	// The sound to play when the attack begins.
	std::shared_ptr<Sound> attackSound;

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

	// The duration of hit stun, in seconds, to deal.
	float hitStun{ 0.f };
};

#endif