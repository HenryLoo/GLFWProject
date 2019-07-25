#include "AttackCollisionSystem.h"

#include "GameEngine.h"

AttackCollisionSystem::AttackCollisionSystem(GameEngine &game,
	std::vector<GameComponent::Physics> &physics,
	std::vector<GameComponent::Attack> &attacks) :
	GameSystem(game, { GameComponent::COMPONENT_PHYSICS,
		GameComponent::COMPONENT_ATTACK }),
	m_physics(physics), m_attacks(attacks)
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
			m_physics[targetId].speed.x += direction * m_attacks[attackId].pattern.knockback.x;
			m_physics[targetId].speed.y += m_attacks[attackId].pattern.knockback.y;
		}
	}
}

void AttackCollisionSystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	// Do nothing.
}