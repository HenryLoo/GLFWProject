#pragma once
#ifndef PrefabLoader_H
#define PrefabLoader_H

#include "ITypeLoader.h"

class PrefabLoader : public ITypeLoader
{
public:
	std::shared_ptr<IAssetType> load(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name);

	// Get the number of streams required for this loader.
	virtual int getNumStreamsRequired() const;
};

#endif