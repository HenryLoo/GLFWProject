#pragma once
#ifndef SpriteSheetLoader_H
#define SpriteSheetLoader_H

#include "TextureLoader.h"
#include "SpriteSheet.h"

#include <unordered_map>

class SpriteSheetLoader : public TextureLoader
{
public:
	SpriteSheetLoader();
	
private:
	virtual std::shared_ptr<IAssetType> loadFromStream(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name, int flag);

	void loadAnimations(const IDataStream::Result &streamedData,
		GLint textureWidth, GLint textureHeight,
		std::unordered_map<std::string, SpriteSheet::SpriteSet> &sprites) const;
};

#endif