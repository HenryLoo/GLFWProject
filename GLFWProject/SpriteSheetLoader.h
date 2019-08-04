#pragma once
#ifndef SpriteSheetLoader_H
#define SpriteSheetLoader_H

#include "TextureLoader.h"

class SpriteSheetLoader : public TextureLoader
{
public:
	std::shared_ptr<IAssetType> load(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name);
	
	// Get the number of streams required for this loader.
	virtual int getNumStreamsRequired() const;
};

#endif