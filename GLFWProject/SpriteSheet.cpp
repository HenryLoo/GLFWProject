#include "SpriteSheet.h"
#include "GameComponent.h"

SpriteSheet::SpriteSheet(GLuint id, GLint width, GLint height, GLint numChannels,
	const std::unordered_map<std::string, SpriteSet> &sprites,  
	std::string name) :
	Texture(id, width, height, numChannels), m_sprites(sprites),
	m_name(name)
{

}

SpriteSheet::SpriteSheet(GLuint id, GLint width, GLint height, GLint numChannels, 
	std::string name) :
	Texture(id, width, height, numChannels), m_name(name)
{

}

bool SpriteSheet::setSprite(const std::string &label, 
	GameComponent::Sprite &output) const
{
	auto it{ m_sprites.find(label) };
	if (it != m_sprites.end())
	{
		// Animation was found, so set it and reset the frame.
		output.currentSprite = it->second;
		output.currentFrame = 0;
		output.currentFrameTime = 0;

		return true;
	}

	return false;
}

bool SpriteSheet::getSprite(const std::string &label, SpriteSet &sprite) const
{
	auto it{ m_sprites.find(label) };
	if (it != m_sprites.end())
	{
		// Animation was found.
		sprite = it->second;

		return true;
	}

	return false;
}

const std::string &SpriteSheet::getName() const
{
	return m_name;
}