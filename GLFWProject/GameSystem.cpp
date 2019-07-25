#include "GameSystem.h"

GameSystem::GameSystem(GameEngine &game,
	const std::set<GameComponent::ComponentType> &components) :
	m_game(game), m_components(components)
{

}

void GameSystem::update(float deltaTime, int numEntities,
	std::vector<unsigned long> &entities)
{
	for (int i = 0; i < numEntities; i++)
	{
		// If the entity doesn't have all the required components,
		// then skip it.
		unsigned long &e{ entities[i] };
		if (!hasComponents(e)) continue;

		// Otherwise, process the entity.
		process(deltaTime, i, e);
	}
}

bool GameSystem::hasComponents(unsigned long &entity)
{
	bool hasComps{ true };
	for (const GameComponent::ComponentType &comp : m_components)
	{
		hasComps = hasComps && GameComponent::hasComponent(entity, comp);
	}

	return hasComps;
}