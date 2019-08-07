#pragma once
#ifndef SpriteSheetLoader_H
#define SpriteSheetLoader_H

#include "TextureLoader.h"

#include <unordered_map>

struct SpriteAnimation;

class SpriteSheetLoader : public TextureLoader
{
public:
	SpriteSheetLoader();
	
private:
	virtual std::shared_ptr<IAssetType> loadFromStream(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name);

	void loadAnimations(const IDataStream::Result &streamedData,
		glm::ivec2 &clipSize,
		std::unordered_map<std::string, SpriteAnimation> &anims) const;
};

#endif