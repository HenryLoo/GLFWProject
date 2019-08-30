#include "Texture.h"

Texture::Texture(GLuint id, GLint width, GLint height, GLint numChannels) :
	m_id(id), m_width(width), m_height(height), m_numChannels(numChannels)
{

}

Texture::~Texture()
{
	glDeleteTextures(1, &m_id);
}

void Texture::bind()
{
	glBindTexture(GL_TEXTURE_2D, m_id);
}

glm::ivec2 Texture::getSize() const
{
	return glm::ivec2(m_width, m_height);
}

GLint Texture::getNumChannels() const
{
	return m_numChannels;
}