#include "AttackCollisionSystem.h"

#include "CharStates.h"
#include "EntityManager.h"
#include "Sound.h"

#include <iostream>

namespace
{
	// A minimal amount of hit stun for when hurt while in the air.
	// This is just for the state machine to detect HURT_AIR's edge condition.
	const float MIN_HIT_STUN{ 0.1f };

	// Max angle in degrees for rotations.
	const float MAX_ROTATION{ 360.f };

	// The minimum amount of vertical knockback against airborne targets.
	const float MIN_KNOCKBACK_Y{ 32.f };

	// The constant to multiply knockback by, to get its hit stop 
	// duration in seconds.
	const float HIT_STOP_MULTIPLIER{ 0.001f };

	// The duration in seconds to set the recently hit timer for players.
	const float RECENTLY_HIT_DURATION{ 4.f };
}

AttackCollisionSystem::AttackCollisionSystem(EntityManager &manager,
	SoLoud::Soloud &soundEngine,
	std::vector<GameComponent::Physics> &physics,
	std::vector<GameComponent::Sprite> &sprites,
	std::vector<GameComponent::Collision> &collisions,
	std::vector<GameComponent::Attack> &attacks,
	std::vector<GameComponent::Character> &characters,
	GameComponent::Player &player) :
	GameSystem(manager, {}),
	m_soundEngine(soundEngine), m_physics(physics), m_sprites(sprites), 
	m_collisions(collisions), m_attacks(attacks), m_characters(characters),
	m_player(player)
{

}

void AttackCollisionSystem::update(float deltaTime, int numEntities,
	std::vector<unsigned long> &entities)
{
	// Handle collisions.
	const std::vector<std::pair<AABBSource, AABBSource>> &collisions{ m_manager.getCollisions() };
	for (const auto &col : collisions)
	{
		// Determine which entity of the collision pair is the attacker.
		AABBSource::Type firstType{ col.first.type };
		AABBSource::Type secondType{ col.second.type };
		bool isFirstAttack{ firstType == AABBSource::Type::Attack };
		bool isSecondAttack{ secondType == AABBSource::Type::Attack };
		int firstId{ isFirstAttack ? m_attacks[col.first.entityId].sourceId : col.first.entityId };
		int secondId{ isSecondAttack ? m_attacks[col.second.entityId].sourceId : col.second.entityId };

		// Only consider collisions between attack and collision boxes of different sources.
		if (isFirstAttack != isSecondAttack && firstId != secondId &&
			(isFirstAttack && m_attacks[col.first.entityId].isEnabled ||
				isSecondAttack && m_attacks[col.second.entityId].isEnabled))
		{
			int targetId{ isFirstAttack ? secondId : firstId };
			int attackId{ isFirstAttack ? firstId : secondId };

			// If the target is already dead, then skip this collision response.
			GameComponent::Character &character{ m_characters[targetId] };
			if (GameComponent::isDead(character))
				continue;

			// If the target is invincible, then skip this collision response.
			if (character.invincibilityTimer > 0.f)
				continue;

			// If the target is fallen, then skip this collision response.
			if (character.states.getState() == CharState::FALLEN)
				continue;

			// If the attack and the target are in the same team, then skip 
			// this collision response.
			GameComponent::Attack &attack{ m_attacks[attackId] };
			if (character.team == attack.team)
				continue;

			// If the target has already been hit, then skip this collision response.
			std::set<int> &hitEntities{ attack.hitEntities };
			if (hitEntities.find(targetId) != hitEntities.end())
				continue;

			// Add to the list of hit entities.
			hitEntities.insert(targetId);

			// Play the hit sound.
			if (attack.pattern.hitSound != nullptr)
				attack.pattern.hitSound->play(m_soundEngine, deltaTime);

			// If the target is airborne, apply a minimum vertical knockback.
			GameComponent::Collision &col{ m_collisions[targetId] };
			GameComponent::Physics &phys{ m_physics[targetId] };
			glm::vec2 knockback{ attack.pattern.knockback };
			if (GameComponent::isInAir(phys, col))
			{
				knockback.y = glm::max(knockback.y, MIN_KNOCKBACK_Y);
			}

			// Apply hit stop to the attacker and target if the attack launches.
			// The duration of hit stop depends on the strength of the knockback.
			float hitStopDuration{ knockback.y * HIT_STOP_MULTIPLIER };
			if (knockback.y > 0.f)
			{
				character.hitStopTimer = hitStopDuration;
				character.isFirstHitStopFrame = true;
				m_characters[attackId].hitStopTimer = hitStopDuration;
			}

			// Ignore hit stun and knockback if the target has super armour.
			bool hasSuperArmour{ false };
			GameComponent::Sprite &spr{ m_sprites[targetId] };
			if (GameComponent::hasComponent(entities[targetId],
				GameComponent::COMPONENT_ATTACK))
			{
				GameComponent::Attack &targetAttack{ m_attacks[targetId] };
				hasSuperArmour = GameComponent::hasSuperArmour(spr, targetAttack);
			}

			if (!hasSuperArmour)
			{
				// Set the hit stun timer.
				float hitStun{ attack.pattern.hitStun };
				if (GameComponent::isInAir(phys, col) || knockback.y != 0)
				{
					// Reset hit stun if knocked into the air.
					hitStun = hitStopDuration + MIN_HIT_STUN;
				}

				spr.isResetAnimation = true;
				character.hitStunTimer = hitStun;

				// Apply knockback to target.
				if (knockback.x != 0.f)
				{
					int direction{ m_physics[attackId].scale.x > 0 ? 1 : -1 };
					phys.speed.x = direction * knockback.x;
				}
				if (knockback.y != 0.f)
				{
					phys.speed.y = knockback.y;
				}
			}

			// Deal damage to target.
			character.health -= attack.pattern.damage;
			character.health = glm::max(0, character.health);

			// If the target is the player, reset the recently hit duration and 
			// accumulate recent health lost.
			if (GameComponent::hasComponent(entities[targetId],
				GameComponent::COMPONENT_PLAYER))
			{
				m_player.recentHealthLost += attack.pattern.damage;
				m_player.recentlyHitTimer = RECENTLY_HIT_DURATION;
			}

			// Create hit spark effect.
			if (!attack.pattern.hitSpark.empty())
			{
				float rotation{ static_cast<float>(rand()) / 
					(static_cast<float>(RAND_MAX / MAX_ROTATION)) };
				m_manager.createEffect(attack.pattern.hitSpark, phys.pos, 
					glm::vec2(1.2f), 255, 255, 255, 255, rotation);
			}

		}
	}
}

void AttackCollisionSystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	// Do nothing.
}