#include "AssetLoader.h"

#include "ITypeLoader.h"
#include "JSONUtilities.h"
#include "SpriteSheet.h"
#include "Room.h"
#include "Script.h"
#include "Shader.h"
#include "Sound.h"
#include "Prefab.h"
#include "Font.h"

#include <json/single_include/nlohmann/json.hpp>

namespace
{
	const std::string ASSETS_FILE{ "assets.json" };

	const std::string LIST_FONTS{ "font" };
	const std::string LIST_PREFABS{ "prefab" };
	const std::string LIST_ROOMS{ "room" };
	const std::string LIST_SCRIPTS{ "script" };
	const std::string LIST_SHADERS{ "shader" };
	const std::string LIST_SOUNDS{ "sound" };
	const std::string LIST_SPRITESHEETS{ "spritesheet" };
	const std::string LIST_TEXTURES{ "texture" };

	const std::string PROPERTY_NAME{ "name" };
	const std::string PROPERTY_FILE{ "file" };

	const std::vector<std::pair<std::string, std::type_index>> ASSET_TYPES
	{
		{ LIST_FONTS, std::type_index(typeid(Font)) },
		{ LIST_PREFABS, std::type_index(typeid(Prefab)) },
		{ LIST_ROOMS, std::type_index(typeid(Room)) },
		{ LIST_SCRIPTS, std::type_index(typeid(Script)) },
		{ LIST_SHADERS, std::type_index(typeid(Shader)) },
		{ LIST_SOUNDS, std::type_index(typeid(Sound)) },
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
		nlohmann::json j;
		*(result[0].stream) >> j;

		// Load list of assets.
		for (auto &type : ASSET_TYPES)
		{
			loadAssetList(j, type.first, type.second);
		}

		std::cout << "AssetLoader::AssetLoader: Assets list done loading\n" << std::endl;
	}
	catch (const nlohmann::json::exception &e)
	{
		std::cout << "AssetLoader::AssetLoader: " << e.what() << std::endl;
	}
}

void AssetLoader::loadAssetList(nlohmann::json json, const std::string &assetLabel,
	const std::type_index &assetType)
{
	// Get listings from json.
	try
	{
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
	catch (const nlohmann::json::exception &e)
	{
		std::cout << "AssetLoader::loadAssetList: " << e.what() << std::endl;
	}
}

void AssetLoader::update(float deltaTime)
{
	// Update each loader.
	for (auto &element : m_loaders)
	{
		element.second->update(deltaTime);
	}
}