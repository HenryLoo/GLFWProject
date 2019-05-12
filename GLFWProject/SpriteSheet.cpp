#include "SpriteSheet.h"

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

void SpriteSheet::getAnimation(const std::string &label, 
	SpriteAnimation &output) const
{
	auto it{ m_animations.find(label) };
	if (it != m_animations.end())
	{
		output = it->second;
	}
}
