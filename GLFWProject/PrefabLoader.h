#pragma once
#ifndef PrefabLoader_H
#define PrefabLoader_H

#include "ITypeLoader.h"

class PrefabLoader : public ITypeLoader
{
private:
	virtual std::shared_ptr<IAssetType> loadFromStream(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name);
};

#endif