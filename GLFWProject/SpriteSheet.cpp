#include "SpriteSheet.h"
#include "GameComponent.h"

SpriteSheet::SpriteSheet(GLuint id, GLint width, GLint height, GLint numChannels,
	const std::unordered_map<std::string, SpriteAnimation> &animations,
	glm::ivec2 clipSize, std::string name) :
	Texture(id, width, height, numChannels), m_animations(animations), 
	m_clipSize(clipSize), m_name(name)
{

}

SpriteSheet::SpriteSheet(GLuint id, GLint width, GLint height, GLint numChannels, 
	glm::ivec2 clipSize, std::string name) :
	Texture(id, width, height, numChannels), m_clipSize(clipSize), m_name(name)
{

}

glm::ivec2 SpriteSheet::getClipSize() const
{
	return m_clipSize;
}

bool SpriteSheet::setAnimation(const std::string &label, 
	GameComponent::Sprite &output) const
{
	auto it{ m_animations.find(label) };
	if (it != m_animations.end())
	{
		// Animation was found, so set it and reset the frame.
		output.currentAnimation = it->second;
		output.currentFrame = 0;
		output.currentFrameTime = 0;

		return true;
	}

	return false;
}

const std::string &SpriteSheet::getName() const
{
	return m_name;
}