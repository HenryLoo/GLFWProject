#pragma once
#ifndef TextureLoader_H
#define TextureLoader_H

#include "ITypeLoader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

class TextureLoader : public ITypeLoader
{
public:
	IAssetType *load(std::iostream *stream, int length, std::string name);

protected:
	bool loadValues(std::iostream *stream, int length,
		GLuint &textureId, GLint &width, GLint &height,
		GLint &numChannels);
};

#endif