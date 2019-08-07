#pragma once
#ifndef ShaderLoader_H
#define ShaderLoader_H

#include "ITypeLoader.h"

class ShaderLoader : public ITypeLoader
{
public:
	ShaderLoader();

private:
	virtual std::shared_ptr<IAssetType> loadFromStream(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name);
};

#endif