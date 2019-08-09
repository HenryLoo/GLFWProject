#pragma once
#ifndef FontLoader_H
#define FontLoader_H

#include "ITypeLoader.h"

class FontLoader : public ITypeLoader
{
private:
	virtual std::shared_ptr<IAssetType> loadFromStream(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name, int flag);
};

#endif