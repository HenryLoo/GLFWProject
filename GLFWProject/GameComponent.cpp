#include "GameComponent.h"

#include <glm/gtc/random.hpp>

bool GameComponent::hasComponent(unsigned long entityCompMask, ComponentType comp)
{
	return entityCompMask & comp;
}

void GameComponent::addComponent(unsigned long &entityCompMask, ComponentType comp)
{
	entityCompMask |= comp;
}

float GameComponent::getFrameDuration(const Sprite &spr)
{
	return spr.currentSprite.clips[spr.currentFrame].duration;
}

int GameComponent::getNumSprites(const Sprite& spr)
{
	return static_cast<int>(spr.currentSprite.clips.size());
}

bool GameComponent::isColliding(const Collision& col)
{
	return (col.isCollidingFloor || col.isCollidingGhost || col.isCollidingSlope);
}

bool GameComponent::isInAir(const Physics& phys, const Collision& col)
{
	bool isCollide{ isColliding(col) };
	return (!isCollide || (isCollide && phys.speed.y == 0.f));
}

bool GameComponent::isOnGround(const Physics& phys, const Collision& col)
{
	bool isCollide{ isColliding(col) };
	return (isCollide && phys.speed.y < 0.f);
}

bool GameComponent::hasHitStop(unsigned long& entityMask, 
	const Character& character)
{
	bool hasHitStop{ false };
	if (GameComponent::hasComponent(entityMask, COMPONENT_CHARACTER))
	{
		hasHitStop = character.hitStopTimer > 0.f && 
			!character.isFirstHitStopFrame;
	}

	return hasHitStop;
}

void GameComponent::updateTimer(float deltaTime, float &timer)
{
	if (timer > 0.f)
	{
		timer -= deltaTime;
		timer = glm::max(0.f, timer);
	}
}

bool GameComponent::isDead(const Character &character)
{
	return character.health == 0;
}

void GameComponent::setActionTimer(Enemy &enemy)
{
	enemy.actionTimer = enemy.actionDuration * glm::linearRand(0.8f, 1.2f);
}