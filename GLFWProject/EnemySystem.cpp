#include "EnemySystem.h"

#include <iostream>

EnemySystem::EnemySystem(EntityManager &manager,
	std::vector<GameComponent::Enemy> &enemies) :
	GameSystem(manager, { GameComponent::COMPONENT_ENEMY }),
	m_enemies(enemies)
{

}

void EnemySystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	GameComponent::Enemy &enemy{ m_enemies[entityId] };

	// Update the action timer.
	GameComponent::updateTimer(deltaTime, enemy.actionTimer);
}