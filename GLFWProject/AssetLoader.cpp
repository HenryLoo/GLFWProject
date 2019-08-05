#include "AssetLoader.h"

#include "ITypeLoader.h"
#include "JSONUtilities.h"
#include "SpriteSheet.h"
#include "Room.h"
#include "Shader.h"

#include <json/single_include/nlohmann/json.hpp>

using json = nlohmann::json;

namespace
{
	const std::string ASSETS_FILE{ "assets.json" };

	const std::string LIST_ROOMS{ "rooms" };
	const std::string LIST_SHADERS{ "shaders" };
	const std::string LIST_SPRITESHEETS{ "spritesheets" };
	const std::string LIST_TEXTURES{ "textures" };

	const std::string PROPERTY_NAME{ "name" };
	const std::string PROPERTY_FILE{ "file" };

	const std::vector<std::pair<std::string, std::type_index>> ASSET_TYPES
	{
		{ LIST_ROOMS, std::type_index(typeid(Room)) },
		{ LIST_SHADERS, std::type_index(typeid(Shader)) },
		{ LIST_SPRITESHEETS, std::type_index(typeid(SpriteSheet)) },
		{ LIST_TEXTURES, std::type_index(typeid(Texture)) }
	};
}

AssetLoader::AssetLoader(IDataStream *stream)
{
	m_stream = std::unique_ptr<IDataStream>(stream);

	// Get the assets list.
	std::vector<IDataStream::Result> result;
	m_stream->getStream({ ASSETS_FILE }, result);

	try
	{
		// Read the contents into json.
		json j;
		*(result[0].stream) >> j;

		// Load list of assets.
		for (auto &type : ASSET_TYPES)
		{
			loadAssetList(j, type.first, type.second);
		}

		std::cout << "AssetLoader::AssetLoader: Assets list done loading\n" << std::endl;
	}
	catch (const nlohmann::json::parse_error &e)
	{
		std::cout << "AssetLoader::AssetLoader: " << e.what() << std::endl;
	}
	catch (const nlohmann::json::invalid_iterator &e)
	{
		std::cout << "AssetLoader::AssetLoader: " << e.what() << std::endl;
	}
	catch (const nlohmann::json::type_error &e)
	{
		std::cout << "AssetLoader::AssetLoader: " << e.what() << std::endl;
	}
	catch (const nlohmann::json::out_of_range &e)
	{
		std::cout << "AssetLoader::AssetLoader: " << e.what() << std::endl;
	}
	catch (const nlohmann::json::other_error &e)
	{
		std::cout << "AssetLoader::AssetLoader: " << e.what() << std::endl;
	}
}

void AssetLoader::loadAssetList(json json, const std::string &assetLabel,
	const std::type_index &assetType)
{
	// Get listings from json.
	if (JSONUtilities::hasEntry(assetLabel, json))
	{
		auto it{ m_assetsList.insert({ assetType, {} }) };
		auto &assets{ it.first->second };

		for (auto &element : json[assetLabel])
		{
			// Get the asset's name.
			std::string name;
			if (JSONUtilities::hasEntry(PROPERTY_NAME, element))
			{
				name = element.at(PROPERTY_NAME).get<std::string>();
			}
			else
			{
				// Skip this listing, since it is invalid.
				continue;
			}

			// Get the asset's files.
			std::vector<std::string> files;
			if (JSONUtilities::hasEntry(PROPERTY_FILE, element))
			{
				auto &file{ element[PROPERTY_FILE] };

				// Asset has multiple files.
				if (file.is_array())
				{
					for (auto &element : file)
					{
						std::string filePath{ element.get<std::string>() };
						files.push_back(assetLabel + "/" + filePath);
					}
				}
				// Asset only has one file.
				else if (file.is_string())
				{
					std::string filePath{ file.get<std::string>() };
					files.push_back(assetLabel + "/" + filePath);
				}
			}
			else
			{
				// Skip this listing, since it is invalid.
				continue;
			}

			// Add the asset to the list.
			assets.insert({ name, files });
		}

		std::cout << "AssetLoader::loadAssetList: List loaded for '" << assetLabel <<
			"', " << assets.size() << " assets" << std::endl;
	}
}