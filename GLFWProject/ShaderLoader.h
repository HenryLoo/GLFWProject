#pragma once
#ifndef ShaderLoader_H
#define ShaderLoader_H

#include "ITypeLoader.h"

class ShaderLoader : public ITypeLoader
{
public:
	std::shared_ptr<IAssetType> load(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name);

	// Get the number of streams required for this loader.
	virtual int getNumStreamsRequired() const;
};

#endif