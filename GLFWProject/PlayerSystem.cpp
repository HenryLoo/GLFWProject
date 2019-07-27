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
	std::vector<GameComponent::Sprite> &sprites,
	std::vector<GameComponent::Weapon> &weapons,
	std::vector<GameComponent::Collision> &collisions,
	std::vector<GameComponent::Attack> &attacks,
	std::vector<GameComponent::Character> &characters):
	GameSystem(game, { GameComponent::COMPONENT_PLAYER,
		GameComponent::COMPONENT_PHYSICS,
		GameComponent::COMPONENT_SPRITE,
		GameComponent::COMPONENT_WEAPON,
		GameComponent::COMPONENT_COLLISION,
		GameComponent::COMPONENT_ATTACK,
		GameComponent::COMPONENT_CHARACTER }),
	m_player(player), m_physics(physics), m_sprites(sprites), 
	m_weapons(weapons), m_collisions(collisions), m_attacks(attacks),
	m_characters(characters)
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
	GameComponent::Sprite &sprite{ m_sprites[entityId] };
	GameComponent::Weapon &weapon{ m_weapons[entityId] };
	GameComponent::Collision &col{ m_collisions[entityId] };
	GameComponent::Attack &attack{ m_attacks[entityId] };
	GameComponent::Character &character{ m_characters[entityId] };
	InputManager *input{ m_game.getInputManager() };

	// Save the previous state.
	character.previousState = character.currentState;
	if (character.previousState.empty())
		character.previousState = CharState::IDLE;

	bool isRunningLeft{ input->isKeyPressing(INPUT_LEFT) };
	bool isRunningRight{ input->isKeyPressing(INPUT_RIGHT) };
	bool isCrouching{ input->isKeyPressing(INPUT_DOWN) };
	bool isRunning{ isRunningLeft != isRunningRight };
	bool isOnGround{ col.isCollidingFloor || col.isCollidingGhost ||
		col.isCollidingSlope };
	bool isJumping{ input->isKeyPressed(INPUT_JUMP) };
	bool isAttacking{ input->isKeyPressed(INPUT_ATTACK) };

	std::string state{ character.currentState };
	if (state.empty())
		state = CharState::IDLE;

	bool isAttackState{ state == CharState::ATTACK || 
		state == CharState::ATTACK_AIR || 
		state == CharState::ATTACK_CROUCH };

	bool isCrouchState{ state == CharState::CROUCH ||
		state == CharState::ATTACK_CROUCH };

	// Handle running.
	// Can't move if crouch key is being pressed.
	// Can only move while attacking if in the air.
	float speed = 0.f;
	float dir = physics.scale.x;
	if (isRunning && (!isOnGround || !isCrouching) && 
		!(isAttackState && isOnGround && state != CharState::ATTACK_AIR))
	{
		// Show running animation if on the ground.
		if (isOnGround)
		{
			// Show turning animation if moving in opposite from the direction 
			// the player is facing.
			if ((physics.scale.x < 0 && isRunningRight) ||
				(physics.scale.x > 0 && isRunningLeft))
			{
				sprite.isResetAnimation = true;
				state = CharState::TURN;
			}
			// Otherwise just start running.
			else if (character.currentState != CharState::RUN &&
				character.currentState != CharState::TURN)
				state = CharState::RUN_START;
		}
		
		speed = 128.f;

		if (isRunningLeft)
		{
			speed *= -1;
			dir = -glm::abs(dir);
		}
		else if (isRunningRight)
		{
			dir = glm::abs(dir);
		}
	}
	physics.speed.x = speed;
	physics.scale.x = dir;

	// Change state to run_stop if the player stops running.
	bool isStopRunningLeft{ input->isKeyReleased(INPUT_LEFT) };
	bool isStopRunningRight{ input->isKeyReleased(INPUT_RIGHT) };
	if ((character.previousState == CharState::RUN ||
		character.previousState == CharState::RUN_START) &&
		(isStopRunningLeft || isStopRunningRight || !isRunning))
	{
		state = CharState::RUN_STOP;
	}

	// While on the ground.
	if (isOnGround)
	{
		// Handle crouching.
		// The player can only crouch while on the ground.
		if (isCrouching && !isAttackState)
		{
			state = CharState::CROUCH;
		}
		// Stopped crouching.
		else if (state == CharState::CROUCH && !isCrouching && !isAttackState)
		{
			state = CharState::CROUCH_STOP;
		}
		// Landed and not running or crouching.
		else if (!isRunning && (character.previousState == CharState::JUMP_DESCEND ||
			character.previousState == CharState::JUMP_PEAK ||
			character.previousState == CharState::ATTACK_AIR))
		{
			state = CharState::JUMP_LAND;
		}

		// If the player landed on the ground, reset the remaining jumps.
		m_player.numRemainingJumps = m_player.numMaxJumps;

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

	// Handle jumping.
	// The player can only jump if there are still remaining jumps.
	if (isJumping && m_player.numRemainingJumps > 0)
	{
		sprite.isResetAnimation = true;
		state = CharState::JUMP_ASCEND;
		m_player.numRemainingJumps--;

		// Dropping down, through ghost platforms.
		if (col.isCollidingGhost && !col.isCollidingFloor && isCrouching)
		{
			physics.pos.y--;
			physics.speed.y = 0;
		}
		// Regular jump.
		else
		{
			physics.speed.y = 256.f;
		}
	}

	// If starting to fall, then change to jump_peak.
	if (physics.speed.y < 0 && !isOnGround &&
		character.currentState != CharState::JUMP_DESCEND &&
		!isAttackState)
	{
		state = CharState::JUMP_PEAK;
	}


	// Handle attacking.
	if (isAttacking)
	{
		if (isOnGround)
		{
			if (!isCrouchState)
				state = CharState::ATTACK;
			else
				state = CharState::ATTACK_CROUCH;
		}
		else
		{
			state = CharState::ATTACK_AIR;
		}
	}

	// Handle natural sprite transitions.
	// If the current frame is over and the animation is a non-looping one 
	// at its last frame.
	float frameDuration{ GameComponent::getFrameDuration(sprite) };
	if (sprite.currentFrameTime >= frameDuration &&
		(!sprite.currentAnimation.isLooping && sprite.currentFrame == sprite.currentAnimation.numSprites - 1))
	{
		const std::string &currentState{ character.currentState };
		if (currentState == CharState::JUMP_PEAK)
			state = CharState::JUMP_DESCEND;
		else if (currentState == CharState::RUN_START)
			state = CharState::RUN;
		else if (currentState == CharState::RUN_STOP)
			state = CharState::ALERT;
		else if (currentState == CharState::JUMP_LAND ||
			currentState == CharState::ATTACK)
			state = CharState::CROUCH_STOP;
		else if (currentState == CharState::ALERT || 
			currentState == CharState::CROUCH_STOP)
			state = CharState::IDLE;
		else if (currentState == CharState::TURN)
			state = CharState::RUN_START;
		else if (currentState == CharState::ATTACK_CROUCH)
			state = CharState::CROUCH;
		else if (currentState == CharState::ATTACK_AIR)
		{
			if (physics.speed.y > 0)
			{
				state = CharState::JUMP_ASCEND;
			}
			else
			{
				state = CharState::JUMP_DESCEND;
			}
		}
	}

	// Queue up the next state to change.
	character.nextState = state;
}