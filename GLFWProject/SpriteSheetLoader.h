#pragma once
#ifndef SpriteSheetLoader_H
#define SpriteSheetLoader_H

#include "TextureLoader.h"

class SpriteSheetLoader : public TextureLoader
{
public:
	IAssetType *load(std::iostream *stream, int length, std::string name);
};

#endif