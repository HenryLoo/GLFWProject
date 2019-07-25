#include "SpriteSystem.h"

#include "Camera.h"
#include "GameEngine.h"
#include "SpriteRenderer.h"

#include <glm/gtx/norm.hpp>

SpriteSystem::SpriteSystem(GameEngine &game, 
	std::vector<GameComponent::Physics> &physics,
	std::vector<GameComponent::Sprite> &sprites,
	std::vector<GameComponent::Weapon> &weapons) :
	GameSystem(game, { GameComponent::COMPONENT_PHYSICS,
		GameComponent::COMPONENT_SPRITE }),
	m_physics(physics), m_sprites(sprites), m_weapons(weapons)
{

}

void SpriteSystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	GameComponent::Physics &phys{ m_physics[entityId] };
	GameComponent::Sprite &sprite{ m_sprites[entityId] };
	
	// Do nothing if the sprite is time-based and the duration is over.
	if (sprite.duration <= 0.f && sprite.hasDuration)
	{
		return;
	}
	
	bool isAlive{ true };
	
	// Decrease the sprite's duration if the sprite is time-limited.
	if (sprite.hasDuration)
		sprite.duration -= deltaTime;
	
	// Update sprite values for this frame.
	if (sprite.duration > 0.0f || !sprite.hasDuration)
	{
		// Update this sprite's values.
		Camera *camera{ m_game.getCamera() };
		sprite.cameraDistance = glm::length2(phys.pos - camera->getPosition());
	
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
	SpriteRenderer *renderer{ m_game.getSpriteRenderer() };
	renderer->addSprite(phys, sprite);

	// Render this entity's weapon only if it exists.
	if (!GameComponent::hasComponent(entityMask, GameComponent::COMPONENT_WEAPON))
	{
		return;
	}

	GameComponent::Weapon &weapon{ m_weapons[entityId] };

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
		SpriteRenderer *renderer{ m_game.getSpriteRenderer() };
		renderer->addSprite(phys, weaponSprite);
	}
}