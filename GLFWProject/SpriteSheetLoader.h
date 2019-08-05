#pragma once
#ifndef SpriteSheetLoader_H
#define SpriteSheetLoader_H

#include "TextureLoader.h"

#include <unordered_map>

struct SpriteAnimation;

class SpriteSheetLoader : public TextureLoader
{
public:
	std::shared_ptr<IAssetType> load(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name);
	
	// Get the number of streams required for this loader.
	virtual int getNumStreamsRequired() const;
	
private:
	void loadAnimations(const IDataStream::Result &streamedData,
		glm::ivec2 &clipSize,
		std::unordered_map<std::string, SpriteAnimation> &anims) const;
};

#endif