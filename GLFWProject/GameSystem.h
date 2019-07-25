#pragma once
#ifndef GameSystem_H
#define GameSystem_H

#include "GameComponent.h"

class GameEngine;

class GameSystem
{
public:
	GameSystem(GameEngine &game, 
		const std::set<GameComponent::ComponentType> &components);

	// Perform this system's updates.
	// This should be called once per game update loop.
	virtual void update(float deltaTime, int numEntities,
		std::vector<unsigned long> &entities);

protected:
	GameEngine &m_game;

	// Hold all relevant component types that this system will use.
	std::set<GameComponent::ComponentType> m_components;

	// Check if a given entity has the components required by the system.
	bool hasComponents(unsigned long &entity);

private:
	// Perform system-specific actions. 
	// This will be called by GameSystem::update() on each entity.
	virtual void process(float deltaTime, int entityId, 
		unsigned long &entityMask) = 0;
};

#endif