#include "SpriteSystem.h"

#include "EntityManager.h"
#include "SpriteRenderer.h"

#include <glm/gtx/norm.hpp>

namespace
{
	const int ALPHA_FADEOUT{ 255 * 2 };
}

SpriteSystem::SpriteSystem(EntityManager &manager,
	SpriteRenderer *sRenderer,
	std::vector<GameComponent::Physics> &physics,
	std::vector<GameComponent::Sprite> &sprites,
	std::vector<GameComponent::Weapon> &weapons) :
	GameSystem(manager, { GameComponent::COMPONENT_PHYSICS,
		GameComponent::COMPONENT_SPRITE }),
	m_sRenderer(sRenderer), m_physics(physics), m_sprites(sprites), 
	m_weapons(weapons)
{

}

void SpriteSystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	GameComponent::Physics &phys{ m_physics[entityId] };
	GameComponent::Sprite &spr{ m_sprites[entityId] };

	// No sprites available, so just skip this.
	if (spr.currentSprite.clips.size() == 0)
		return;

	// Update sprite values for this frame.
	if (spr.isPersistent || !spr.isDead)
	{
		// Update this sprite's values.
		/*Camera *camera{ m_manager.getGameEngine().getCamera() };
		sprite.cameraDistance = glm::length2(phys.pos - camera->getPosition());*/
	
		// Update this sprite's animation.
		spr.currentFrameTime += deltaTime;
		float frameDuration{ GameComponent::getFrameDuration(spr) };

		// Process the next frame if the current frame is over and the 
		// animation is not a non-looping one at its last frame.
		bool isFrameEnded{ spr.currentFrameTime >= frameDuration && frameDuration > 0.f };
		int numSprites{ GameComponent::getNumSprites(spr) };
		bool isLastFrame{ spr.currentFrame == numSprites - 1 };
		if (isFrameEnded &&
			!(!spr.currentSprite.isLooping && isLastFrame))
		{
			// Check if deltaTime has accumulated a value greater than the
			// frame's duration, and then find how many frames should be
			// processed.
			float leftoverTime = spr.currentFrameTime - frameDuration;
			do
			{
				spr.currentFrameTime = leftoverTime;
				spr.currentFrame++;
				if (spr.currentFrame >= numSprites)
					spr.currentFrame = 0;
	
				// Get how long this new frame lasts and see if there is still
				// enough leftover time to process it.
				frameDuration = GameComponent::getFrameDuration(spr);
				leftoverTime -= frameDuration;
			} while (leftoverTime >= frameDuration);
		}
		// Otherwise, if the animation over and it is not persistent, then flag
		// the entity as dead.
		else if (!spr.isPersistent && isLastFrame && isFrameEnded)
		{
			spr.isDead = true;
		}
	}
	// The sprite is time-limited and this is its last frame.
	else
	{
		// Fade out the sprite.
		if (spr.a > 0)
		{
			spr.a = static_cast<unsigned char>(glm::max(0.f, spr.a - ALPHA_FADEOUT * deltaTime));
		}
		// Done fading out, so delete the entity.
		else
		{
			// Set distance to camera to be minimum value so that
			// it will be placed at the back of the sprites array when sorted.
			spr.cameraDistance = -1.0f;

			// Flag the entity for deletion.
			m_manager.deleteEntity(entityId);
			return;
		}
	}
	
	// Update the renderer's array of sprites.
	m_sRenderer->addSprite(phys, spr);

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
		weaponSprite.currentSprite = weapon.currentSprite;
		weaponSprite.currentFrame = spr.currentFrame;
		weaponSprite.cameraDistance = spr.cameraDistance;
		weaponSprite.r = spr.r;
		weaponSprite.g = spr.g;
		weaponSprite.b = spr.b;
		weaponSprite.a = spr.a;

		// Update the renderer's array of sprites.
		m_sRenderer->addSprite(phys, weaponSprite);
	}
}