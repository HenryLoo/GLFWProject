#include "AttackCollisionSystem.h"

#include "CharStates.h"
#include "EffectTypes.h"
#include "EntityManager.h"

namespace
{
	// A minimal amount of hit stun for when hurt while in the air.
	// This is just for the state machine to detect HURT_AIR's edge condition.
	const float MIN_HIT_STUN{ 0.1f };
}

AttackCollisionSystem::AttackCollisionSystem(EntityManager &manager,
	std::vector<GameComponent::Physics> &physics,
	std::vector<GameComponent::Sprite> &sprites,
	std::vector<GameComponent::Collision> &collisions,
	std::vector<GameComponent::Attack> &attacks,
	std::vector<GameComponent::Character> &characters) :
	GameSystem(manager, {}),
	m_physics(physics), m_sprites(sprites), m_collisions(collisions),
	m_attacks(attacks), m_characters(characters)
{

}

void AttackCollisionSystem::update(float deltaTime, int numEntities,
	std::vector<unsigned long> &entities)
{
	// Handle collisions.
	const std::vector<std::pair<AABBSource, AABBSource>> &collisions{ m_manager.getCollisions() };
	for (const auto &col : collisions)
	{
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

			// If the target has already been hit, then skip this collision response.
			std::set<int> &hitEntities{ m_attacks[attackId].hitEntities };
			if (hitEntities.find(targetId) != hitEntities.end())
			{
				continue;
			}

			// Add to the list of hit entities.
			hitEntities.insert(targetId);

			// Set the hit stun timer.
			glm::vec2 knockback{ m_attacks[attackId].pattern.knockback };
			float hitStun{ 1.f }; // TODO: change fixed value
			GameComponent::Collision &collision{ m_collisions[targetId] };
			if ((!collision.isColliding() || (collision.isColliding() && m_physics[targetId].speed.y == 0.f)) ||
				knockback.y != 0)
			{
				// Reset hit stun if knocked into the air.
				hitStun = MIN_HIT_STUN;
			}

			// Apply knockback to target.
			int direction{ m_physics[attackId].scale.x > 0 ? 1 : -1 };
			GameComponent::Physics &phys{ m_physics[targetId] };
			phys.speed.x += direction * knockback.x;
			phys.speed.y = knockback.y;

			m_sprites[targetId].isResetAnimation = true;
			m_characters[targetId].hitStunTimer = hitStun;

			// Create hit spark effect.
			m_manager.createEffect(EffectType::HIT_SPARK, phys.pos, glm::vec2(1.f),
				255, 255, 255, 255);
		}
	}
}

void AttackCollisionSystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	// Do nothing.
}