#pragma once
#ifndef TextureLoader_H
#define TextureLoader_H

#include "ITypeLoader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

class TextureLoader : public ITypeLoader
{
public:
	std::shared_ptr<IAssetType> load(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name);

	// Get the number of streams required for this loader.
	virtual int getNumStreamsRequired() const;

protected:
	bool loadValues(const IDataStream::Result &streamedData,
		GLuint &textureId, GLint &width, GLint &height,
		GLint &numChannels);
};

#endif