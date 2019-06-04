#include "GameSystem.h"
#include "GameEngine.h"
#include "SpriteRenderer.h"
#include "InputManager.h"
#include "CharStates.h"

#include <glm/gtx/norm.hpp>

#include <iostream>

namespace
{
	const float COLLISION_THRESHOLD{ 0.01f };
}

bool GameSystem::updatePhysics(float deltaTime, GameComponent::Physics &physics)
{
	// Update this component's values.
	physics.speed += glm::vec3(0.0f, -256.f * deltaTime, 0.0f);
	physics.speed.y = glm::max(-128.f, physics.speed.y);
	//physics.pos += physics.speed * deltaTime;

	return true;
}

bool GameSystem::updateSprite(float deltaTime, SpriteRenderer *renderer,
	glm::vec3 cameraPos, GameComponent::Sprite &sprite, 
	GameComponent::Physics &physics)
{
	// Do nothing if the sprite is time-based and the duration is over.
	if (sprite.duration <= 0.f && sprite.hasDuration)
	{
		return false;
	}

	bool isAlive{ true };

	// Decrease the sprite's duration if the sprite is time-limited.
	if (sprite.hasDuration)
		sprite.duration -= deltaTime;

	// Update sprite values for this frame.
	if (sprite.duration > 0.0f || !sprite.hasDuration)
	{
		// Update this sprite's values.
		sprite.cameraDistance = glm::length2(physics.pos - cameraPos);

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
	renderer->addSprite(physics, sprite);

	return isAlive;
}

bool GameSystem::updatePlayer(InputManager *input, 
	GameComponent::Player &player, GameComponent::Physics &physics, 
	GameComponent::Sprite &sprite, GameComponent::AABB &aabb)
{
	// Save the previous state.
	player.previousState = player.currentState;
	if (player.previousState.empty())
		player.previousState = PlayerState::IDLE;

	bool isRunningLeft{ input->isKeyPressing(INPUT_LEFT) };
	bool isRunningRight{ input->isKeyPressing(INPUT_RIGHT) };
	bool isRunning{ isRunningLeft != isRunningRight };

	std::string state{ player.currentState };
	if (state.empty())
		state = PlayerState::IDLE;

	// Flag for if the animation should be reset.
	// This is used for setting the same state.
	bool isResetAnimation{ false };

	// Only one running key is pressed.
	float speed = 0.f;
	float dir = physics.scale.x;
	if (isRunning)
	{
		// Show running animation if on the ground.
		if (aabb.isCollidingBottom)
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

	// If the player landed on the ground, reset the remaining jumps.
	if (aabb.isCollidingBottom)
	{
		// Landed and not running.
		if (!isRunning && (player.previousState == PlayerState::JUMP_DESCEND || 
			player.previousState == PlayerState::JUMP_PEAK))
			state = PlayerState::IDLE;

		player.numRemainingJumps = player.numMaxJumps;
	}
	// Remove 1 remaining jump if walking off a ledge.
	else
	{
		player.numRemainingJumps = glm::min(player.numRemainingJumps, 
			player.numMaxJumps - 1);
	}

	// Handle jumping.
	// The player can only jump if there are still remaining jumps.
	if (input->isKeyPressed(INPUT_JUMP) && player.numRemainingJumps > 0)
	{
		isResetAnimation = true;
		state = PlayerState::JUMP_ASCEND;
		physics.speed.y = 192.f;
		player.numRemainingJumps--;
	}

	// If starting to fall, then change to jump_peak.
	if (physics.speed.y < 0)
	{
		if (player.currentState != PlayerState::JUMP_DESCEND)
			state = PlayerState::JUMP_PEAK;
	}

	// Handle natural sprite transitions.
	// If the current frame is over and the animation is a non-looping one 
	// at its last frame.
	float frameDuration{ GameComponent::getFrameDuration(sprite) };
	if (sprite.currentFrameTime >= frameDuration &&
		(!sprite.currentAnimation.isLooping && sprite.currentFrame == sprite.currentAnimation.numSprites - 1))
	{
		if (player.currentState == PlayerState::JUMP_PEAK)
			state = PlayerState::JUMP_DESCEND;
		else if (player.currentState == PlayerState::RUN_START)
			state = PlayerState::RUN;
		else if (player.currentState == PlayerState::RUN_STOP)
			state = PlayerState::ALERT;
		else if (player.currentState == PlayerState::ALERT)
			state = PlayerState::IDLE;
		else if (player.currentState == PlayerState::TURN)
			state = PlayerState::RUN_START;
	}

	// Change the sprite's state if it is a different one.
	if (player.currentState != state || isResetAnimation)
	{
		player.currentState = state;
		sprite.spriteSheet->setAnimation(state, sprite);
	}

	return true;
}

bool GameSystem::updateRoomCollision(float deltaTime,
	GameComponent::Physics &physics, GameComponent::AABB &aabb, Room *room)
{
	// Reset collision flags.
	aabb.isCollidingBottom = false;
	aabb.isCollidingTop = false;

	glm::vec2 speed{ physics.speed };
	glm::vec2 halfSize{ aabb.halfSize };
	glm::vec2 pos{ physics.pos.x + aabb.offset.x, 
		physics.pos.y + aabb.offset.y };

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
					physics.pos.x = tileEdgePos + direction * halfSize.x - aabb.offset.x;
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
			physics.pos.x += physics.speed.x * deltaTime;
	}

	// Check for vertical collisions.
	if (speed.y != 0)
	{
		// Extend the halfsize of the entity to see how much distance it will
		// cover this frame.
		float distY = halfSize.y + abs(speed.y) * deltaTime;
		glm::ivec2 currentTile{ room->getTileCoord(pos) };

		// 1 = moving down, -1 = moving up.
		int direction{ speed.y < 0 ? 1 : -1 };
		bool &isColliding{ direction == 1 ? aabb.isCollidingBottom : aabb.isCollidingTop };

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
		while (currentTileY != tileRangeToCheck.y)
		{
			// Check all tiles at this height.
			for (int i = minXTile.x; i <= maxXTile.x; ++i)
			{
				glm::ivec2 thisTileCoord{ i, currentTileY };
				TileType type{ room->getTileType(thisTileCoord) };
				if (type == TILE_WALL)
				{
					float tileEdgePos{ room->getTilePos(thisTileCoord).y
						+ direction * Room::TILE_SIZE / 2.f };
					physics.pos.y = tileEdgePos + direction * halfSize.y - aabb.offset.y;
					isColliding = true;

					// Collision was found, so there is no need to keep checking.
					break;
				}
			}

			// Collision was found, so there is no need to keep checking. 
			if (isColliding)
				break;

			currentTileY -= direction;
		}

		// If not colliding, then just apply velocity as usual.
		if (!isColliding)
			physics.pos.y += physics.speed.y * deltaTime;
		// Otherwise, stop moving.
		else
			physics.speed.y = 0;
	}

	return true;
}