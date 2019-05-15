#include "SpriteSheet.h"
#include "GameComponent.h"

SpriteSheet::SpriteSheet(const std::string &filePath,
	const std::unordered_map<std::string, SpriteAnimation> &animations,
	glm::vec2 clipSize) :
	Texture(filePath), m_animations(animations), m_clipSize(clipSize)
{

}

SpriteSheet::~SpriteSheet()
{

}

glm::vec2 SpriteSheet::getClipSize() const
{
	return m_clipSize;
}

void SpriteSheet::setAnimation(const std::string &label, 
	GameComponent::Sprite &output) const
{
	auto it{ m_animations.find(label) };
	if (it != m_animations.end())
	{
		// Animation was found, so set it and reset the frame.
		output.currentAnimation = it->second;
		output.currentFrame = 0;
		output.currentFrameTime = 0;
	}
}
