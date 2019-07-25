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
	std::vector<GameComponent::Attack> &attacks):
	GameSystem(game, { GameComponent::COMPONENT_PLAYER,
		GameComponent::COMPONENT_PHYSICS,
		GameComponent::COMPONENT_SPRITE,
		GameComponent::COMPONENT_WEAPON,
		GameComponent::COMPONENT_COLLISION,
		GameComponent::COMPONENT_ATTACK }),
	m_player(player), m_physics(physics), m_sprites(sprites), 
	m_weapons(weapons), m_collisions(collisions), m_attacks(attacks)
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
	InputManager *input{ m_game.getInputManager() };

	// Save the previous state.
	m_player.previousState = m_player.currentState;
	if (m_player.previousState.empty())
		m_player.previousState = PlayerState::IDLE;

	bool isRunningLeft{ input->isKeyPressing(INPUT_LEFT) };
	bool isRunningRight{ input->isKeyPressing(INPUT_RIGHT) };
	bool isCrouching{ input->isKeyPressing(INPUT_DOWN) };
	bool isRunning{ isRunningLeft != isRunningRight };
	bool isOnGround{ col.isCollidingFloor || col.isCollidingGhost ||
		col.isCollidingSlope };
	bool isJumping{ input->isKeyPressed(INPUT_JUMP) };
	bool isAttacking{ input->isKeyPressed(INPUT_ATTACK) };

	std::string state{ m_player.currentState };
	if (state.empty())
		state = PlayerState::IDLE;

	bool isAttackState{ state == PlayerState::ATTACK || 
		state == PlayerState::ATTACK_AIR || 
		state == PlayerState::ATTACK_CROUCH };

	bool isCrouchState{ state == PlayerState::CROUCH ||
		state == PlayerState::ATTACK_CROUCH };

	// Flag for if the animation should be reset.
	// This is used for setting the same state.
	bool isResetAnimation{ false };

	// Handle running.
	// Can't move if crouch key is being pressed.
	// Can only move while attacking if in the air.
	float speed = 0.f;
	float dir = physics.scale.x;
	if (isRunning && (!isOnGround || !isCrouching) && 
		!(isAttackState && isOnGround && state != PlayerState::ATTACK_AIR))
	{
		// Show running animation if on the ground.
		if (isOnGround)
		{
			// Show turning animation if moving in opposite from the direction 
			// the player is facing.
			if ((physics.scale.x < 0 && isRunningRight) ||
				(physics.scale.x > 0 && isRunningLeft))
			{
				isResetAnimation = true;
				state = PlayerState::TURN;
			}
			// Otherwise just start running.
			else if (m_player.currentState != PlayerState::RUN &&
				m_player.currentState != PlayerState::TURN)
				state = PlayerState::RUN_START;
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
	if ((m_player.previousState == PlayerState::RUN ||
		m_player.previousState == PlayerState::RUN_START) &&
		(isStopRunningLeft || isStopRunningRight || !isRunning))
	{
		state = PlayerState::RUN_STOP;
	}

	// While on the ground.
	if (isOnGround)
	{
		// Handle crouching.
		// The player can only crouch while on the ground.
		if (isCrouching && !isAttackState)
		{
			state = PlayerState::CROUCH;
		}
		// Stopped crouching.
		else if (state == PlayerState::CROUCH && !isCrouching && !isAttackState)
		{
			state = PlayerState::CROUCH_STOP;
		}
		// Landed and not running or crouching.
		else if (!isRunning && (m_player.previousState == PlayerState::JUMP_DESCEND ||
			m_player.previousState == PlayerState::JUMP_PEAK ||
			m_player.previousState == PlayerState::ATTACK_AIR))
		{
			state = PlayerState::JUMP_LAND;
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
		isResetAnimation = true;
		state = PlayerState::JUMP_ASCEND;
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
			physics.speed.y = 192.f;
		}
	}

	// If starting to fall, then change to jump_peak.
	if (physics.speed.y < 0 && !isOnGround &&
		m_player.currentState != PlayerState::JUMP_DESCEND &&
		!isAttackState)
	{
		state = PlayerState::JUMP_PEAK;
	}


	// Handle attacking.
	if (isAttacking)
	{
		if (isOnGround)
		{
			if (!isCrouchState)
				state = PlayerState::ATTACK;
			else
				state = PlayerState::ATTACK_CROUCH;
		}
		else
		{
			state = PlayerState::ATTACK_AIR;
		}
	}

	// Handle natural sprite transitions.
	// If the current frame is over and the animation is a non-looping one 
	// at its last frame.
	float frameDuration{ GameComponent::getFrameDuration(sprite) };
	if (sprite.currentFrameTime >= frameDuration &&
		(!sprite.currentAnimation.isLooping && sprite.currentFrame == sprite.currentAnimation.numSprites - 1))
	{
		const std::string &currentState{ m_player.currentState };
		if (currentState == PlayerState::JUMP_PEAK)
			state = PlayerState::JUMP_DESCEND;
		else if (currentState == PlayerState::RUN_START)
			state = PlayerState::RUN;
		else if (currentState == PlayerState::RUN_STOP)
			state = PlayerState::ALERT;
		else if (currentState == PlayerState::JUMP_LAND ||
			currentState == PlayerState::ATTACK)
			state = PlayerState::CROUCH_STOP;
		else if (currentState == PlayerState::ALERT || 
			currentState == PlayerState::CROUCH_STOP)
			state = PlayerState::IDLE;
		else if (currentState == PlayerState::TURN)
			state = PlayerState::RUN_START;
		else if (currentState == PlayerState::ATTACK_CROUCH)
			state = PlayerState::CROUCH;
		else if (currentState == PlayerState::ATTACK_AIR)
		{
			if (physics.speed.y > 0)
			{
				state = PlayerState::JUMP_ASCEND;
			}
			else
			{
				state = PlayerState::JUMP_DESCEND;
			}
		}
	}

	// Change the sprite's state if it is a different one.
	if (m_player.currentState != state || isResetAnimation)
	{
		m_player.currentState = state;
		sprite.spriteSheet->setAnimation(state, sprite);

		// Create a temporary sprite component to get the weapon's 
		// sprite animation. Set the animation if it exists.
		GameComponent::Sprite weaponSprite;
		weapon.isVisible = weapon.spriteSheet->setAnimation(state, weaponSprite);
		if (weapon.isVisible)
		{
			weapon.currentAnimation = weaponSprite.currentAnimation;
		}

		// Set the attack pattern for this state if it exists.
		auto it{ m_player.attackPatterns.find(state) };
		if (it != m_player.attackPatterns.end())
		{
			attack.pattern = it->second;
		}
		else
		{
			attack.pattern = {};
		}

		attack.hitEntities.clear();
	}
}