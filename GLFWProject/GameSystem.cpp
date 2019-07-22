#include "GameSystem.h"

#include "SpriteRenderer.h"
#include "InputManager.h"
#include "CharStates.h"
#include "Room.h"
#include "UIRenderer.h"

#include <glm/gtx/norm.hpp>

#include <cmath>
#include <iostream>

namespace
{
	// Distance in pixels to subtract from collision boxes to
	// prevent overlapping between horizontal and vertical tests.
	const float COLLISION_THRESHOLD{ 1.f };

	// Duration of time in seconds to allow for illusion jumps.
	const float ILLUSION_JUMP_DURATION{ 0.1f };
}

void GameSystem::updatePhysics(float deltaTime, int numEntities,
	std::vector<unsigned long> &entities,
	std::vector<GameComponent::Physics> &physics)
{
	for (int i = 0; i < numEntities; i++)
	{
		unsigned long &e{ entities[i] };
		bool hasPhysics{ GameComponent::hasComponent(e, GameComponent::COMPONENT_PHYSICS) };
		if (!hasPhysics) continue;

		// Update this component's values.
		physics[i].speed += glm::vec3(0.0f, -256.f * deltaTime, 0.0f);
		physics[i].speed.y = glm::max(-144.f, physics[i].speed.y);
	}
}

void GameSystem::updateSprite(float deltaTime, int numEntities,
	std::vector<unsigned long> &entities,
	SpriteRenderer *renderer, glm::vec3 cameraPos,
	std::vector<GameComponent::Sprite> &sprites,
	std::vector<GameComponent::Physics> &physics)
{
	for (int i = 0; i < numEntities; i++)
	{
		unsigned long &e{ entities[i] };
		bool hasPhysics{ GameComponent::hasComponent(e, GameComponent::COMPONENT_PHYSICS) };
		bool hasSprite{ GameComponent::hasComponent(e, GameComponent::COMPONENT_SPRITE) };
		if (!hasPhysics || !hasSprite) continue;

		GameComponent::Sprite &sprite{ sprites[i] };
		GameComponent::Physics &phys{ physics[i] };

		// Do nothing if the sprite is time-based and the duration is over.
		if (sprite.duration <= 0.f && sprite.hasDuration)
		{
			//return false;
		}

		bool isAlive{ true };

		// Decrease the sprite's duration if the sprite is time-limited.
		if (sprite.hasDuration)
			sprite.duration -= deltaTime;

		// Update sprite values for this frame.
		if (sprite.duration > 0.0f || !sprite.hasDuration)
		{
			// Update this sprite's values.
			sprite.cameraDistance = glm::length2(phys.pos - cameraPos);

			// Update this sprite's animation.
			sprite.currentFrameTime += deltaTime;
			float frameDuration{ GameComponent::getFrameDuration(sprite) };

			// Process the next frame if the current frame is over and the 
			// animation is not a non-looping one at its last frame.
			if (sprite.currentFrameTime >= frameDuration &&
				!(!sprite.currentAnimation.isLooping && sprite.currentFrame == sprite.currentAnimation.numSprites - 1))
			{
				// Check if deltaTime has accumulated a value greater than the
				// frame's duration, and then find how many frames should be
				// processed.
				float leftoverTime = sprite.currentFrameTime - frameDuration;
				do
				{
					sprite.currentFrameTime = leftoverTime;
					sprite.currentFrame++;
					if (sprite.currentFrame >= sprite.currentAnimation.numSprites)
						sprite.currentFrame = 0;

					// Get how long this new frame lasts and see if there is still
					// enough leftover time to process it.
					frameDuration = GameComponent::getFrameDuration(sprite);
					leftoverTime -= frameDuration;
				} while (leftoverTime >= frameDuration);
			}
		}
		// The sprite is time-limited and this is its last frame.
		else
		{
			// Set distance to camera to be minimum value so that
			// it will be placed at the back of the sprites array when sorted.
			sprite.cameraDistance = -1.0f;

			// Delete the entity.
			isAlive = false;
		}

		// Update the renderer's array of sprites.
		renderer->addSprite(phys, sprite);
	}
}

void GameSystem::updatePlayer(float deltaTime, unsigned long(&playerEntity), 
	InputManager *input,
	GameComponent::Player &player, GameComponent::Physics &physics, 
	GameComponent::Sprite &sprite, GameComponent::Weapon &weapon, 
	GameComponent::Collision &col, GameComponent::Attack &attack)
{
	bool hasPlayer{ GameComponent::hasComponent(playerEntity, GameComponent::COMPONENT_PLAYER) };
	bool hasPhysics{ GameComponent::hasComponent(playerEntity, GameComponent::COMPONENT_PHYSICS) };
	bool hasSprite{ GameComponent::hasComponent(playerEntity, GameComponent::COMPONENT_SPRITE) };
	bool hasWeapon{ GameComponent::hasComponent(playerEntity, GameComponent::COMPONENT_WEAPON) };
	bool hasCollision{ GameComponent::hasComponent(playerEntity, GameComponent::COMPONENT_COLLISION) };
	bool hasAttack{ GameComponent::hasComponent(playerEntity, GameComponent::COMPONENT_ATTACK) };
	if (!hasPlayer || !hasPhysics || !hasSprite || !hasWeapon || !hasCollision || !hasAttack) return;

	// Save the previous state.
	player.previousState = player.currentState;
	if (player.previousState.empty())
		player.previousState = PlayerState::IDLE;

	bool isRunningLeft{ input->isKeyPressing(INPUT_LEFT) };
	bool isRunningRight{ input->isKeyPressing(INPUT_RIGHT) };
	bool isCrouching{ input->isKeyPressing(INPUT_DOWN) };
	bool isRunning{ isRunningLeft != isRunningRight };
	bool isOnGround{ col.isCollidingFloor || col.isCollidingGhost ||
		col.isCollidingSlope };
	bool isJumping{ input->isKeyPressed(INPUT_JUMP) };
	bool isAttacking{ input->isKeyPressed(INPUT_ATTACK) };

	std::string state{ player.currentState };
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
			else if (player.currentState != PlayerState::RUN &&
				player.currentState != PlayerState::TURN)
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
	if ((player.previousState == PlayerState::RUN || 
		player.previousState == PlayerState::RUN_START) &&
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
		else if (!isRunning && (player.previousState == PlayerState::JUMP_DESCEND ||
			player.previousState == PlayerState::JUMP_PEAK ||
			player.previousState == PlayerState::ATTACK_AIR))
		{
			state = PlayerState::JUMP_LAND;
		}

		// If the player landed on the ground, reset the remaining jumps.
		player.numRemainingJumps = player.numMaxJumps;

		// Reset illusion jump timer.
		player.illusionJumpTimer = 0.f;
	}
	else
	{
		// Remove 1 remaining jump if walking off a ledge.
		// Allow for a short duration of time before doing this, so
		// that it feels more forgiving when jumping off ledges.
		if (player.illusionJumpTimer >= ILLUSION_JUMP_DURATION)
		{
			player.numRemainingJumps = glm::min(player.numRemainingJumps,
				player.numMaxJumps - 1);
		}
		else
		{
			player.illusionJumpTimer += deltaTime;
		}

		// Reset fall speed if walking off a ledge.
		if (col.wasOnGround && physics.speed.y < 0)
		{
			physics.speed.y = 0.f;
		}
	}

	// Handle jumping.
	// The player can only jump if there are still remaining jumps.
	if (isJumping && player.numRemainingJumps > 0)
	{
		isResetAnimation = true;
		state = PlayerState::JUMP_ASCEND;
		player.numRemainingJumps--;

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
		player.currentState != PlayerState::JUMP_DESCEND &&
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
		const std::string &currentState{ player.currentState };
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
	if (player.currentState != state || isResetAnimation)
	{
		player.currentState = state;
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
		auto it{ player.attackPatterns.find(state) };
		if (it != player.attackPatterns.end())
		{
			attack.pattern = it->second;
		}
		else
		{
			attack.pattern = {};
		}
	}
}

void GameSystem::updateRoomCollision(float deltaTime, int numEntities,
	std::vector<unsigned long> &entities,
	std::vector<GameComponent::Physics> &physics,
	std::vector<GameComponent::Collision> &collisions,
	Room *room)
{
	for (int i = 0; i < numEntities; i++)
	{
		unsigned long &e{ entities[i] };
		bool hasPhysics{ GameComponent::hasComponent(e, GameComponent::COMPONENT_PHYSICS) };
		bool hasCollision{ GameComponent::hasComponent(e, GameComponent::COMPONENT_COLLISION) };
		if (!hasPhysics || !hasCollision) continue;

		GameComponent::Physics &phys{ physics[i] };
		GameComponent::Collision &col{ collisions[i] };

			// Reset collision flags.
		col.wasOnGround = col.isCollidingFloor || col.isCollidingGhost || col.isCollidingSlope;
		col.isCollidingFloor = false;
		col.isCollidingGhost = false;

		const AABB &aabb{ col.aabb };
		glm::vec2 speed{ phys.speed };
		glm::vec2 halfSize{ aabb.halfSize };
		glm::vec2 pos{ phys.pos.x + aabb.offset.x,
			phys.pos.y + aabb.offset.y };

		// Check for horizontal collisions.
		if (speed.x != 0)
		{
			// Extend the halfsize of the entity to see how much distance it will
			// cover this frame.
			float distX = halfSize.x + abs(speed.x) * deltaTime;
			glm::ivec2 currentTile{ room->getTileCoord(pos) };

			// 1 = moving left, -1 = moving right.
			int direction{ speed.x < 0 ? 1 : -1 };

			// Get the vertical tile bounds at the maximum X-distance.
			float maxDistX{ pos.x - direction * distX };
			float halfSizeY{ halfSize.y - COLLISION_THRESHOLD };
			glm::vec2 minYPos{ maxDistX, pos.y - halfSizeY };
			glm::vec2 maxYPos{ maxDistX, pos.y + halfSizeY };
			glm::ivec2 minYTile{ room->getTileCoord(minYPos) };
			glm::ivec2 maxYTile{ room->getTileCoord(maxYPos) };

			// If the entity is on a slope, ignore horizontal collisions for the
			// bottom-most tile. This will stop the collision box from
			// "catching" onto the tile when transitioning from slope onto floor.
			if (col.isCollidingSlope)
			{
				minYTile.y = glm::min(minYTile.y + 1, maxYTile.y);
			}

			// Set up bounds for the loop.
			// minYTile.x == maxYTile.x should be true.
			glm::ivec2 tileRangeToCheck{ currentTile.x, minYTile.x };
			tileRangeToCheck.y -= direction;

			// Check all potential collisions before applying velocity.
			// We check in order of closest to furthest tiles.
			bool isColliding{ false };
			int currentTileX{ tileRangeToCheck.x };
			while (currentTileX != tileRangeToCheck.y)
			{
				// Check all tiles at this height.
				for (int i = minYTile.y; i <= maxYTile.y; ++i)
				{
					glm::ivec2 thisTileCoord{ currentTileX, i };
					TileType type{ room->getTileType(thisTileCoord) };
					if (type == TILE_WALL)
					{
						float tileEdgePos{ room->getTilePos(thisTileCoord).x
							+ direction * Room::TILE_SIZE / 2.f };
						phys.pos.x = tileEdgePos + direction * halfSize.x - aabb.offset.x;
						isColliding = true;

						// Collision was found, so there is no need to keep checking.
						break;
					}
				}

				// Collision was found, so there is no need to keep checking. 
				if (isColliding)
					break;

				currentTileX -= direction;
			}

			// If not colliding, then just apply velocity as usual.
			if (!isColliding)
				phys.pos.x += phys.speed.x * deltaTime;
		}

		// Reset the colliding-slope flag after checking for horizontal collisions,
		// because we need to check it to properly transition between slopes and
		// floor tiles.
		col.isCollidingSlope = false;

		// Check for vertical collisions.
		if (speed.y != 0)
		{
			// Extend the halfsize of the entity to see how much distance it will
			// cover this frame.
			float distY = halfSize.y + abs(speed.y) * deltaTime;
			glm::ivec2 currentTile{ room->getTileCoord(pos) };

			// 1 = moving down, -1 = moving up.
			int direction{ speed.y < 0 ? 1 : -1 };

			// Get the horizontal tile bounds at the maximum Y-distance.
			float maxDistY{ pos.y - direction * distY };
			float halfSizeX{ halfSize.x - COLLISION_THRESHOLD };
			glm::vec2 minXPos{ pos.x - halfSizeX, maxDistY };
			glm::vec2 maxXPos{ pos.x + halfSizeX, maxDistY };
			glm::ivec2 minXTile{ room->getTileCoord(minXPos) };
			glm::ivec2 maxXTile{ room->getTileCoord(maxXPos) };

			// Set up bounds for the loop.
			// minXTile.y == maxXTile.y should be true.
			glm::ivec2 tileRangeToCheck{ currentTile.y, minXTile.y };
			tileRangeToCheck.y -= direction;

			// Check all potential collisions before applying velocity.
			// We check in order of closest to furthest tiles.
			int currentTileY{ tileRangeToCheck.x };

			// The entity's new y-position if it has a vertical collision.
			float newYPos{ -1 };

			// Flag for if the entity has almost fallen onto a slope.
			// Since slopes are shorter than regular tlies, we need this flag to check if
			// the entity is within the slope's tile, but not yet colliding with it.
			bool isAlmostSlope{ false };

			while (currentTileY != tileRangeToCheck.y)
			{
				// Check all tiles at this height.
				for (int i = minXTile.x; i <= maxXTile.x; ++i)
				{
					glm::ivec2 thisTileCoord{ i, currentTileY };
					TileType type{ room->getTileType(thisTileCoord) };

					// The edge of the tile to check entity collision against.
					glm::vec2 thisTilePos{ room->getTilePos(thisTileCoord) };
					const static int tileHalfSize{ Room::TILE_SIZE / 2 };
					float tileEdgePos{ thisTilePos.y + direction * tileHalfSize };

					// Check for collisions against slopes first.
					if (Room::isSlope(type) && speed.y <= 0)
					{
						// Get the distance from the left edge of the tile to the entity's position.
						float slopeRad{ atanf((float)Room::SLOPE_HEIGHT / Room::TILE_SIZE) };
						float xDist{ phys.pos.x - (thisTilePos.x - tileHalfSize) };

						// Not close enough onto the slope yet.
						if (xDist < 0 || xDist > Room::TILE_SIZE)
						{
							continue;
						}

						// Adjust y-distance to displace the entity, based on the type of slope.
						float yDist{ tanf(slopeRad) * xDist };

						if (type == TILE_SLOPE_LEFT_UPPER || type == TILE_SLOPE_LEFT_LOWER)
							yDist = Room::TILE_SIZE - yDist;

						if (type == TILE_SLOPE_RIGHT_UPPER)
							yDist += Room::SLOPE_HEIGHT;
						else if (type == TILE_SLOPE_LEFT_LOWER)
							yDist -= Room::SLOPE_HEIGHT;

						tileEdgePos = thisTilePos.y - tileHalfSize + yDist;

						// Slopes are shorter than regular tiles, so we need to check more precisely
						// for collisions.
						if ((phys.pos.y - halfSize.y + aabb.offset.y + speed.y * deltaTime) > tileEdgePos)
						{
							isAlmostSlope = true;
							continue;
						}

						col.isCollidingSlope = true;

						// Set the new y-position if it isn't set already, or if it is at a lower
						// y-position than the current one to set.
						float yPosToSet = tileEdgePos + halfSize.y - aabb.offset.y;
						newYPos = newYPos == -1 ? yPosToSet : glm::min(newYPos, yPosToSet);
					}
					// If the tile is a wall or the entity is colliding against the
					// top edge of a ghost platform.
					// Unlike horizontal collisions, we don't break early here because
					// we need to set all possible collision flags.
					else if (type == TILE_WALL || (type == TILE_GHOST && speed.y <= 0 &&
						(phys.pos.y - halfSize.y + aabb.offset.y) >= tileEdgePos))
					{
						// Set the new y-position if it isn't set already, or if it is at a lower
						// y-position than the current one to set.
						float yPosToSet = tileEdgePos + direction * halfSize.y - aabb.offset.y;
						newYPos = newYPos == -1 ? yPosToSet : glm::min(newYPos, yPosToSet);

						if (type == TILE_WALL)
						{
							col.isCollidingFloor = true;
						}
						else if (type == TILE_GHOST)
						{
							col.isCollidingGhost = true;
						}
					}
				}

				// Collision was found, so there is no need to keep checking. 
				if ((col.isCollidingFloor || col.isCollidingGhost || col.isCollidingSlope))
				{
					// If the entity is within a slope's tile but not yet 
					// colliding with it, then discard any floor collisions that 
					// would have occurred. Any floor collisions would have been 
					// at the same tile y-position as the slope. Since slopes are 
					// shorter than regular tiles, this would incorrectly set the 
					// new y-position to the floor, even though it should be at the
					// slope.
					if (isAlmostSlope && !col.isCollidingSlope)
					{
						col.isCollidingFloor = false;
					}

					break;
				}

				currentTileY -= direction;
			}

			// If not colliding, then just apply velocity as usual.
			if (!col.isCollidingFloor && !col.isCollidingGhost && !col.isCollidingSlope)
			{
				phys.pos.y += phys.speed.y * deltaTime;
			}
			// Colliding against ceiling.
			else if (col.isCollidingFloor && phys.speed.y > 0)
			{
				phys.speed.y = 0;
			}
			else
			{
				phys.pos.y = newYPos;
			}

			// Round to two decimal places to reduce sprite artifacts.
			phys.pos *= 100;
			phys.pos = glm::round(phys.pos);
			phys.pos /= 100;
		}
	}
}

void GameSystem::updateWeapon(float deltaTime, int numEntities,
	std::vector<unsigned long> &entities,
	SpriteRenderer *renderer, glm::vec3 cameraPos,
	std::vector<GameComponent::Sprite> &sprites,
	std::vector<GameComponent::Physics> &physics,
	std::vector<GameComponent::Weapon> &weapons)
{
	for (int i = 0; i < numEntities; i++)
	{
		unsigned long &e{ entities[i] };
		bool hasPhysics{ GameComponent::hasComponent(e, GameComponent::COMPONENT_PHYSICS) };
		bool hasSprite{ GameComponent::hasComponent(e, GameComponent::COMPONENT_SPRITE) };
		bool hasWeapon{ GameComponent::hasComponent(e, GameComponent::COMPONENT_WEAPON) };
		if (!hasPhysics || !hasSprite || !hasWeapon) continue;

		GameComponent::Sprite &sprite{ sprites[i] };
		GameComponent::Physics &phys{ physics[i] };
		GameComponent::Weapon &weapon{ weapons[i] };

		if (weapon.isVisible)
		{
			// Create a temporary sprite component to hold the weapon 
			// sprite's values.
			GameComponent::Sprite weaponSprite;
			weaponSprite.spriteSheet = weapon.spriteSheet;
			weaponSprite.currentAnimation = weapon.currentAnimation;
			weaponSprite.currentFrame = sprite.currentFrame;
			weaponSprite.cameraDistance = sprite.cameraDistance;
			weaponSprite.r = sprite.r;
			weaponSprite.g = sprite.g;
			weaponSprite.b = sprite.b;
			weaponSprite.a = sprite.a;

			// Update the renderer's array of sprites.
			renderer->addSprite(phys, weaponSprite);
		}
	}
}

bool GameSystem::updateAttack(float deltaTIme, GameComponent::Sprite &sprite,
	GameComponent::Attack &attack, GameComponent::Physics &physics,
	int playerId, const std::vector<int> &enemyIds,
	std::vector<GameComponent::Physics> &targetPhysics,
	std::vector<GameComponent::Collision> &targetCols)
{
	attack.isEnabled = (sprite.currentFrame >= attack.pattern.frameRange.x &&
		sprite.currentFrame <= attack.pattern.frameRange.y);

	if (attack.isEnabled)
	{
		//glm::vec2 atkOrigin{ physics.pos.x + physics.scale.x * attack.pattern.offset.x,
		//	physics.pos.y + physics.scale.y * attack.pattern.offset.y };
		//float atkLeftBound{ atkOrigin.x - attack.pattern.halfSize.x };
		//float atkRightBound{ atkOrigin.x + attack.pattern.halfSize.x };
		//float atkBottomBound{ atkOrigin.y - attack.pattern.halfSize.y };
		//float atkTopBound{ atkOrigin.y + attack.pattern.halfSize.y };

		// If this was a player's attack.
		if (attack.source == playerId)
		{
			// Check collisions against all existing enemies.
			for (int enemyId : enemyIds)
			{
				// TODO: AABB collision testing is O(n^2)... try to find a better way to do this.
			}
		}
	}

	return true;
}

void GameSystem::updateDebug(int numEntities,
	std::vector<unsigned long> &entities,
	std::vector<GameComponent::Physics> &physics,
	std::vector<GameComponent::Collision> &collisions,
	std::vector<GameComponent::Attack> &attacks,
	UIRenderer *uRenderer)
{
	for (int i = 0; i < numEntities; i++)
	{
		unsigned long &e{ entities[i] };
		bool hasPhysics{ GameComponent::hasComponent(e, GameComponent::COMPONENT_PHYSICS) };
		bool hasCollision{ GameComponent::hasComponent(e, GameComponent::COMPONENT_COLLISION) };
		bool hasAttack{ GameComponent::hasComponent(e, GameComponent::COMPONENT_ATTACK) };
		if (!hasPhysics || !hasCollision) continue;

		GameComponent::Physics &phys{ physics[i] };
		GameComponent::Collision &col{ collisions[i] };

		uRenderer->addBox(phys, col.aabb, 0, 255, 0, 100);

		if (!hasAttack) continue;

		GameComponent::Attack &atk{ attacks[i] };
		uRenderer->addBox(phys, atk.pattern.aabb, 0, 0, 255, 100);
	}
}