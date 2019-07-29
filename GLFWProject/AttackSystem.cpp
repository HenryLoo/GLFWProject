#include "AttackSystem.h"

AttackSystem::AttackSystem(GameEngine &game,
	std::vector<GameComponent::Sprite> &sprites,
	std::vector<GameComponent::Attack> &attacks) :
	GameSystem(game, { GameComponent::COMPONENT_SPRITE,
		GameComponent::COMPONENT_ATTACK }),
	m_sprites(sprites), m_attacks(attacks)
{

}

void AttackSystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	GameComponent::Sprite &sprite{ m_sprites[entityId] };
	GameComponent::Attack &attack{ m_attacks[entityId] };

	// Update whether attacks are enabled or not.
	// Attacks are enabled if the current sprite frame is within the specified
	// frame range and the frame range consists of at least 1 frame.
	attack.isEnabled = ((sprite.currentFrame >= attack.pattern.frameRange.x &&
		sprite.currentFrame <= attack.pattern.frameRange.y) && 
		(attack.pattern.frameRange.y - attack.pattern.frameRange.x > 0));
}