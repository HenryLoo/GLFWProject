#include "PrefabLoader.h"

#include "Prefab.h"
#include <fstream>

std::shared_ptr<IAssetType> PrefabLoader::loadFromStream(
	const std::vector<IDataStream::Result> &streams,
	const std::string &name, int flag)
{
	const IDataStream::Result &theResult{ streams[0] };
	try
	{
		// Read the contents into json.
		nlohmann::json j;
		*(theResult.stream) >> j;
	
		std::shared_ptr<Prefab> prefab{ std::make_shared<Prefab>(j) };
		if (prefab != nullptr)
		{
			std::cout << "PrefabLoader::loadFromStream: Loaded '" << name << "'\n" << std::endl;
			return prefab;
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		std::cout << "PrefabLoader::loadFromStream: " << e.what() << std::endl;
	}
	
	return nullptr;
}