#pragma once
#ifndef Texture_H
#define Texture_H

#include "IAssetType.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

class Texture : public IAssetType
{
public:
	Texture(GLuint id, GLint width, GLint height, GLint numChannels);
	virtual ~Texture();

	// Bind to this texture.
	void bind();

	// Get the dimensions of the texture.
	glm::ivec2 getSize() const;

protected:
	// This texture's id.
	GLuint m_id;

	// This texture's dimensions.
	GLint m_width;
	GLint m_height;

	// This texture's number of colour channels.
	GLint m_numChannels;
};

#endif