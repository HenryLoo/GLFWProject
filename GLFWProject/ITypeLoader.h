#pragma once
#ifndef ITypeLoader_H
#define ITypeLoader_H

#include <iostream>

class IAssetType;

// Load a specific type of data from a stream.
class ITypeLoader
{
public:
	virtual IAssetType *load(std::iostream *stream, int length) = 0;
};

#endif