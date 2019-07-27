#include "AttackCollisionSystem.h"

#include "CharStates.h"
#include "GameEngine.h"

AttackCollisionSystem::AttackCollisionSystem(GameEngine &game,
	std::vector<GameComponent::Physics> &physics,
	std::vector<GameComponent::Sprite> &sprites,
	std::vector<GameComponent::Collision> &collisions,
	std::vector<GameComponent::Attack> &attacks,
	std::vector<GameComponent::Character> &characters) :
	GameSystem(game, {}),
	m_physics(physics), m_sprites(sprites), m_collisions(collisions),
	m_attacks(attacks), m_characters(characters)
{

}

void AttackCollisionSystem::update(float deltaTime, int numEntities,
	std::vector<unsigned long> &entities)
{
	// Handle collisions.
	const std::vector<std::pair<AABBSource, AABBSource>> &collisions{ m_game.getCollisions() };
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

			// Apply knockback to target.
			int direction{ m_physics[attackId].scale.x > 0 ? 1 : -1 };
			glm::vec2 knockback{ m_attacks[attackId].pattern.knockback };
			m_physics[targetId].speed.x += direction * knockback.x;
			m_physics[targetId].speed.y = knockback.y;

			// Change the target's state to HURT and set hit stun timer.
			// If the target is in the air or if the attack has vertical
			// knockback, then set the state to HURT_AIR instead.
			std::string hurtState{ CharState::HURT };
			float hitStun{ 1.f }; // TODO: change fixed value
			GameComponent::Collision &collision{ m_collisions[targetId] };
			if ((!collision.isCollidingFloor && !collision.isCollidingGhost &&
				!collision.isCollidingSlope) || knockback.y != 0)
			{
				// Reset hit stun if knocked into the air.
				hitStun = 0.f;
				hurtState = CharState::HURT_AIR;
			}

			m_characters[targetId].nextState = hurtState;
			m_sprites[targetId].isResetAnimation = true;
			m_characters[targetId].hitStunTimer = hitStun;
		}
	}
}

void AttackCollisionSystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	// Do nothing.
}