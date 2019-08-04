#include "AssetLoader.h"

#include "ITypeLoader.h"

AssetLoader::AssetLoader(IDataStream *stream)
{
	m_stream = std::unique_ptr<IDataStream>(stream);
}