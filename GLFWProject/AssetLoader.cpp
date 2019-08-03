#include "AssetLoader.h"

#include "ITypeLoader.h"

AssetLoader::AssetLoader(IDataStream *stream)
{
	m_stream = std::unique_ptr<IDataStream>(stream);
}

void AssetLoader::registerLoader(std::string key, ITypeLoader *loader)
{
	m_loaders.insert({ key, std::unique_ptr<ITypeLoader>(loader) });
}

IAssetType *AssetLoader::load(std::string type, std::string name)
{
	auto it{ m_loaders.find(type) };
	if (it == m_loaders.end())
	{
		std::cout << "AssetLoader::load: loader of type '" << type << "' not found" << std::endl;
		return nullptr;
	}

	// Get the stream and pass it to the loader.
	std::iostream *stream{ m_stream->getStream(type, name) };
	int length{ m_stream->getLength() };
	return it->second->load(stream, length, name);
}