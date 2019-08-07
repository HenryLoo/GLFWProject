#include "PrefabLoader.h"

#include "Prefab.h"

using json = nlohmann::json;

std::shared_ptr<IAssetType> PrefabLoader::loadFromStream(
	const std::vector<IDataStream::Result> &streams,
	const std::string &name)
{
	const IDataStream::Result &theResult{ streams[0] };
	try
	{
		// Read the contents into json.
		json j;
		*(theResult.stream) >> j;
	
		std::shared_ptr<Prefab> prefab{ std::make_shared<Prefab>(j) };
		if (prefab != nullptr)
		{
			std::cout << "PrefabLoader::load: Loaded '" << name << "'\n" << std::endl;
			return prefab;
		}
	}
	catch (const nlohmann::json::parse_error &e)
	{
		std::cout << "PrefabLoader::load: " << e.what() << std::endl;
	}
	catch (const nlohmann::json::invalid_iterator &e)
	{
		std::cout << "PrefabLoader::load: " << e.what() << std::endl;
	}
	catch (const nlohmann::json::type_error &e)
	{
		std::cout << "PrefabLoader::load: " << e.what() << std::endl;
	}
	catch (const nlohmann::json::out_of_range &e)
	{
		std::cout << "PrefabLoader::load: " << e.what() << std::endl;
	}
	catch (const nlohmann::json::other_error &e)
	{
		std::cout << "PrefabLoader::load: " << e.what() << std::endl;
	}
	
	return nullptr;
}