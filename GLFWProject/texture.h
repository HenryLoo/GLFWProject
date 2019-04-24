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
	GLuint id;

	// This texture's dimensions.
	GLint width;
	GLint height;

	// This texture's number of colour channels.
	GLint numChannels;
};

#endif