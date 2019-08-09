#pragma once
#ifndef TextureLoader_H
#define TextureLoader_H

#include "ITypeLoader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

class TextureLoader : public ITypeLoader
{
protected:
	bool loadValues(const IDataStream::Result &streamedData,
		GLuint &textureId, GLint &width, GLint &height,
		GLint &numChannels);

private:
	virtual std::shared_ptr<IAssetType> loadFromStream(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name, int flag);
};

#endif