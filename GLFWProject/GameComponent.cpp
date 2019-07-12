#include "GameComponent.h"
#include "SpriteSheet.h"

bool GameComponent::hasComponent(unsigned long entityCompMask, ComponentType comp)
{
	return entityCompMask & comp;
}

void GameComponent::addComponent(unsigned long &entityCompMask, ComponentType comp)
{
	entityCompMask |= comp;
}

float GameComponent::getFrameDuration(const Sprite &sprite)
{
	int numDurations{ static_cast<int>(sprite.currentAnimation.durations.size()) };

	// If there are more frames than durations, just return the last duration value.
	return sprite.currentFrame < numDurations ?
			sprite.currentAnimation.durations[sprite.currentFrame] :
			sprite.currentAnimation.durations.back();
}