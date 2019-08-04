#pragma once
#ifndef ITypeLoader_H
#define ITypeLoader_H

#include "IDataStream.h"

#include <iostream>
#include <memory>

class IAssetType;

// Load a specific type of data from a stream.
class ITypeLoader
{
public:
	// Returns a pointer to the loaded asset.
	// If the asset could not be loaded, return nullptr.
	virtual std::shared_ptr<IAssetType> load(
		const std::vector<IDataStream::Result> &streams, 
		const std::string &name) = 0;

	// Get the number of streams required for this loader.
	virtual int getNumStreamsRequired() const = 0;
};

#endif