#include "PlayerSystem.h"

#include "CharStates.h"
#include "GameEngine.h"

namespace
{
	// Duration of time in seconds to allow for illusion jumps.
	const float ILLUSION_JUMP_DURATION{ 0.1f };
}

PlayerSystem::PlayerSystem(GameEngine &game,
	GameComponent::Player &player,
	std::vector<GameComponent::Physics> &physics,
	std::vector<GameComponent::Collision> &collisions) :
	GameSystem(game, { GameComponent::COMPONENT_PLAYER,
		GameComponent::COMPONENT_PHYSICS,
		GameComponent::COMPONENT_COLLISION }),
	m_player(player), m_physics(physics), m_collisions(collisions)
{

}

void PlayerSystem::update(float deltaTime, int numEntities,
	std::vector<unsigned long> &entities)
{
	int playerId{ m_game.getPlayerId() };
	unsigned long &e{ entities[playerId] };
	if (!hasComponents(e)) return;

	process(deltaTime, playerId, e);
}

void PlayerSystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	GameComponent::Physics &physics{ m_physics[entityId] };
	GameComponent::Collision &col{ m_collisions[entityId] };

	// While on the ground.
	if (col.isColliding() && physics.speed.y < 0.f)
	{
		// If the player landed on the ground, reset the remaining jumps and evades.
		m_player.numRemainingJumps = m_player.numMaxJumps;
		m_player.numRemainingEvades = m_player.numMaxEvades;

		// Reset illusion jump timer.
		m_player.illusionJumpTimer = 0.f;
	}
	else
	{
		// Remove 1 remaining jump if walking off a ledge.
		// Allow for a short duration of time before doing this, so
		// that it feels more forgiving when jumping off ledges.
		if (m_player.illusionJumpTimer >= ILLUSION_JUMP_DURATION)
		{
			m_player.numRemainingJumps = glm::min(m_player.numRemainingJumps,
				m_player.numMaxJumps - 1);
		}
		else
		{
			m_player.illusionJumpTimer += deltaTime;
		}

		// Reset fall speed if walking off a ledge.
		if (col.wasOnGround && physics.speed.y < 0)
		{
			physics.speed.y = 0.f;
		}
	}

	// Update evade timer.
	if (m_player.evadeTimer > 0)
	{
		m_player.evadeTimer -= deltaTime;
		m_player.evadeTimer = glm::max(0.f, m_player.evadeTimer);
	}
}