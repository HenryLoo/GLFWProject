#pragma once
#ifndef ITypeLoader_H
#define ITypeLoader_H

#include <iostream>

class IAssetType;

// Load a specific type of data from a stream.
class ITypeLoader
{
public:
	// Returns a pointer to the loaded asset.
	// If the asset could not be loaded, return nullptr.
	virtual IAssetType *load(std::iostream *stream, int length, std::string name) = 0;
};

#endif