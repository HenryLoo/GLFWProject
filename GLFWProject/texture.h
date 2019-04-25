#pragma once
#ifndef Texture_H
#define Texture_H

#include <glad/glad.h>

#include <string>

class Texture
{
public:
	Texture(const std::string &filePath);
	~Texture();

	// Bind to this texture.
	void bind();

private:
	// This texture's id.
	GLuint m_id;

	// This texture's dimensions.
	GLint m_width;
	GLint m_height;

	// This texture's number of colour channels.
	GLint m_numChannels;
};

#endif