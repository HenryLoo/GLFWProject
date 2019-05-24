#include "GameSystem.h"
#include "GameEngine.h"
#include "SpriteRenderer.h"
#include "InputManager.h"

#include <glm/gtx/norm.hpp>

#include <iostream>

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
		float frameDuration{ sprite.currentAnimation.durations[sprite.currentFrame] };

		// Process the next frame if the current frame is over and the 
		// animation is not a non-looping one at its last frame.
		if (sprite.currentFrameTime >= frameDuration &&
			!(!sprite.isLooping && sprite.currentFrame == sprite.currentAnimation.numSprites - 1))
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
				frameDuration = sprite.currentAnimation.durations[sprite.currentFrame];
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
	GameComponent::Sprite &sprite)
{
	bool isRunningLeft{ input->isKeyPressing(INPUT_LEFT) };
	bool isRunningRight{ input->isKeyPressing(INPUT_RIGHT) };
	bool isRunning{ isRunningLeft != isRunningRight };
	std::string state{ "idle" };

	// Only one running key is pressed.
	float speed = 0.f;
	float dir = physics.scale.x;
	if (isRunning)
	{
		state = "run";
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

	// Handle jumping.
	if (input->isKeyPressed(INPUT_JUMP))
	{
		physics.speed.y = 256.f;
	}

	// Change the sprite's state if it is a different one.
	if (player.currentState != state)
	{
		player.currentState = state;
		sprite.spriteSheet->setAnimation(state, sprite);
	}

	return true;
}

bool GameSystem::updateRoomCollision(float deltaTime,
	GameComponent::Physics &physics, GameComponent::AABB &aabb, Room *room)
{
	glm::vec2 speed{ physics.speed.y };

	// Check for vertical collisions.
	if (speed.y != 0)
	{
		// Extend the halfsize of the entity to see how much distance it will
		// cover this frame.
		float halfSize{ aabb.halfSize.y };
		float distY = halfSize + abs(speed.y) * deltaTime;
		glm::ivec2 currentTile{ room->getTileCoord(physics.pos) };

		// 1 = moving down, -1 = moving up.
		int direction{ speed.y < 0 ? 1 : -1 };
		glm::vec2 maxDistPos{ physics.pos.x, physics.pos.y - direction * distY };
		glm::ivec2 maxDistTile{ room->getTileCoord(maxDistPos) };

		// Set up bounds for the loop.
		glm::ivec2 tileRangeToCheck{ currentTile.y, maxDistTile.y };
		tileRangeToCheck.y -= direction;

		// Check all potential collisions before applying velocity.
		// We check in order of closest to furthest tiles.
		bool isColliding{ false };
		int currentTileY{ tileRangeToCheck.x };
		while (currentTileY != tileRangeToCheck.y)
		{
			glm::ivec2 thisTileCoord{ currentTile.x, currentTileY };
			TileType type{ room->getTileType(thisTileCoord) };
			if (type == TILE_WALL)
			{
				float tileEdgePos{ room->getTilePos(thisTileCoord).y 
					+ direction * Room::TILE_SIZE / 2.f };
				physics.pos.y = tileEdgePos + direction * halfSize;
				isColliding = true;

				// Collision was found, so there is no need to keep checking.
				break;
			}

			currentTileY -= direction;
		}

		// If not colliding, then just apply velocity as usual.
		if (!isColliding)
		{
			physics.pos.y += physics.speed.y * deltaTime;
		}
	}

	physics.pos.x += physics.speed.x * deltaTime;

	return true;
}