#pragma once
#ifndef ITypeLoader_H
#define ITypeLoader_H

#include "IDataStream.h"

#include <iostream>
#include <memory>
#include <unordered_map>

class IAssetType;

// Load a specific type of data from a stream.
class ITypeLoader
{
public:
	// Check the cache for a loaded asset. If it's not found,
	// then load it from the stream.
	// If the asset could not be loaded, return nullptr.
	std::shared_ptr<IAssetType> loadFromCache(
		const std::string &name);

	// Load an asset from a stream and cache it, before returning a pointer 
	// to it.
	// If the asset could not be loaded, return nullptr.
	std::shared_ptr<IAssetType> load(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name);

	// Get the number of streams required for this loader.
	int getNumStreamsRequired() const;

	void update(float deltaTime);

protected:
	// Hold the number of streams that this loader needs.
	// This is equal to the number of files to be loaded for the asset.
	int m_numStreamsRequired{ 1 };

private:
	// Type-specific file-loading.
	// This should be implemented by the subclass.
	virtual std::shared_ptr<IAssetType> loadFromStream(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name) = 0;

	// Hold all loaded assets for a temporary duration of time.
	// This is to avoid loading multiple copies of the same asset.
	std::unordered_map<std::string, std::shared_ptr<IAssetType>>  m_cache;

	// Elapsed time in seconds.
	// When this reaches a threshold, the loader will check whether or not to 
	// clear the cache.
	float m_clearCacheTime;
};

#endif