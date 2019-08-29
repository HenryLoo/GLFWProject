#pragma once
#ifndef MusicLoader_H
#define MusicLoader_H

#include "ITypeLoader.h"

class MusicLoader : public ITypeLoader
{
private:
	virtual std::shared_ptr<IAssetType> loadFromStream(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name, int flag);
};

#endif