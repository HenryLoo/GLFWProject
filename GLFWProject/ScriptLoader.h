#pragma once
#ifndef ScriptLoader_H
#define ScriptLoader_H

#include "ITypeLoader.h"

class ScriptLoader : public ITypeLoader
{
private:
	virtual std::shared_ptr<IAssetType> loadFromStream(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name, int flag);
};

#endif