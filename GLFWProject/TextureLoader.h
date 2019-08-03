#pragma once
#ifndef TextureLoader_H
#define TextureLoader_H

#include "ITypeLoader.h"

class TextureLoader : public ITypeLoader
{
public:
	IAssetType *load(std::iostream *stream, int length);
};

#endif