#pragma once
#ifndef AssetLoader_H
#define AssetLoader_h

#include "IDataStream.h"

#include <memory>
#include <unordered_map>
#include <string>

class IAssetType;
class ITypeLoader;

class AssetLoader
{
public:
	AssetLoader(IDataStream *stream);

	// Register a loader with a given type key.
	void registerLoader(std::string key, ITypeLoader *loader);

	// Load an asset by its asset type and name, through its
	// relevant loader.
	IAssetType *load(std::string type, std::string name);

private:
	// Hold registered loaders for all asset types.
	std::unordered_map<std::string, ITypeLoader*> m_loaders;

	// Hold the stream to get data from.
	std::unique_ptr<IDataStream> m_stream;
};

#endif