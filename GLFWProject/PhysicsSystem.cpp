#include "PhysicsSystem.h"

#include <glm/glm.hpp>

namespace
{
	const float GRAVITY{ -480.f };
	const float FRICTION{ -256.f };
}

PhysicsSystem::PhysicsSystem(GameEngine &game, 
	std::vector<GameComponent::Physics> &physics,
	std::vector<GameComponent::Collision> &collisions) :
	GameSystem(game, { GameComponent::COMPONENT_PHYSICS, 
		GameComponent::COMPONENT_COLLISION }), 
	m_physics(physics), m_collisions(collisions)
{

}

void PhysicsSystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	GameComponent::Physics &phys{ m_physics[entityId] };
	GameComponent::Collision &col{ m_collisions[entityId] };
	
	// Update this component's values.
	phys.speed.y += (GRAVITY * deltaTime);
	phys.speed.y = glm::max(GRAVITY / 3.f, phys.speed.y);
	
	// Only decelerate horizontally if on the ground.
	if ((col.isCollidingFloor || col.isCollidingGhost || col.isCollidingSlope) &&
		phys.speed.x != 0.f)
	{
		float decel{ phys.speed.x > 0 ? FRICTION : -FRICTION };
		phys.speed.x += (decel * deltaTime);
	
		// Round to 0.
		if ((phys.speed.x > 0.f && phys.speed.x < 1.f) || 
			(phys.speed.x < 0.f && phys.speed.x > -1.f))
		{
			phys.speed.x = 0.f;
		}
	}
}