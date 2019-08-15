#include "AttackCollisionSystem.h"

#include "CharStates.h"
#include "EntityManager.h"
#include "Sound.h"

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
}

AttackCollisionSystem::AttackCollisionSystem(EntityManager &manager,
	SoLoud::Soloud &soundEngine,
	std::vector<GameComponent::Physics> &physics,
	std::vector<GameComponent::Sprite> &sprites,
	std::vector<GameComponent::Collision> &collisions,
	std::vector<GameComponent::Attack> &attacks,
	std::vector<GameComponent::Character> &characters) :
	GameSystem(manager, {}),
	m_soundEngine(soundEngine), m_physics(physics), m_sprites(sprites), 
	m_collisions(collisions), m_attacks(attacks), m_characters(characters)
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
			{
				continue;
			}

			// If the target has already been hit, then skip this collision response.
			GameComponent::Attack &attack{ m_attacks[attackId] };
			std::set<int> &hitEntities{ attack.hitEntities };
			if (hitEntities.find(targetId) != hitEntities.end())
			{
				continue;
			}

			// Add to the list of hit entities.
			hitEntities.insert(targetId);

			// Play the hit sound.
			attack.pattern.hitSound->play(m_soundEngine);

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

			// Set the hit stun timer.
			float hitStun{ 1.f }; // TODO: change fixed value
			if (GameComponent::isInAir(phys, col) || knockback.y != 0)
			{
				// Reset hit stun if knocked into the air.
				hitStun = hitStopDuration + MIN_HIT_STUN;
			}

			m_sprites[targetId].isResetAnimation = true;
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

			// Deal damage to target.
			character.health -= attack.pattern.damage;
			character.health = glm::max(0, character.health);

			// Create hit spark effect.
			if (!attack.pattern.hitSpark.empty())
			{
				float rotation{ static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / MAX_ROTATION)) };
				m_manager.createEffect(attack.pattern.hitSpark, phys.pos, glm::vec2(1.2f),
					255, 255, 255, 255, rotation);
			}

		}
	}
}

void AttackCollisionSystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	// Do nothing.
}