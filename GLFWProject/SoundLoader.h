#pragma once
#ifndef SoundLoader_H
#define SoundLoader_H

#include "ITypeLoader.h"

class SoundLoader : public ITypeLoader
{
private:
	virtual std::shared_ptr<IAssetType> loadFromStream(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name, int flag);
};

#endif