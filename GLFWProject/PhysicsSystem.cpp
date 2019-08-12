#include "PhysicsSystem.h"

#include <glm/glm.hpp>

namespace
{
	const float GRAVITY{ -480.f };
	const float FRICTION{ -720.f };
}

PhysicsSystem::PhysicsSystem(EntityManager &manager,
	std::vector<GameComponent::Physics> &physics,
	std::vector<GameComponent::Collision> &collisions,
	std::vector<GameComponent::Character> &characters) :
	GameSystem(manager, { GameComponent::COMPONENT_PHYSICS, 
		GameComponent::COMPONENT_COLLISION }), 
	m_physics(physics), m_collisions(collisions), m_characters(characters)
{

}

void PhysicsSystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	// Skip this if the entity is hit stopped.
	GameComponent::Character &character{ m_characters[entityId] };
	if (GameComponent::hasHitStop(entityMask, character))
		return;

	GameComponent::Physics &phys{ m_physics[entityId] };
	GameComponent::Collision &col{ m_collisions[entityId] };

	// Update this component's values.
	if (phys.hasGravity)
	{
		phys.speed.y += (GRAVITY * deltaTime);
		phys.speed.y = glm::max(GRAVITY / 3.f, phys.speed.y);
	}
	
	// Only decelerate horizontally if on the ground or if a horizontal
	// collision occurs.
	if ((phys.hasFriction && GameComponent::isOnGround(phys, col) &&
		phys.speed.x != 0.f) || col.isCollidingHorizontal)
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