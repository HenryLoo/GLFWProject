#include "DebugSystem.h"

#include "GameEngine.h"

DebugSystem::DebugSystem(GameEngine &game,
	std::vector<GameComponent::Physics> &physics,
	std::vector<GameComponent::Collision> &collisions,
	std::vector<GameComponent::Attack> &attacks) :
	GameSystem(game, { GameComponent::COMPONENT_PHYSICS,
		GameComponent::COMPONENT_COLLISION }),
	m_physics(physics), m_collisions(collisions), m_attacks(attacks)
{

}

void DebugSystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	GameComponent::Physics &phys{ m_physics[entityId] };
	GameComponent::Collision &col{ m_collisions[entityId] };

	UIRenderer *renderer{ m_game.getUIRenderer() };
	renderer->addBox(phys, col.aabb, 0, 255, 0, 100);

	// Draw the hit box for this entity's attack only if it exists.
	if (!GameComponent::hasComponent(entityMask, GameComponent::COMPONENT_ATTACK))
	{
		return;
	}

	GameComponent::Attack &attack{ m_attacks[entityId] };
	renderer->addBox(phys, attack.pattern.aabb, 0, 0, 255, 100);
}