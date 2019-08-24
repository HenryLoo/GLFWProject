#include "Room.h"

#include "Texture.h"
#include "AssetLoader.h"
#include "JSONUtilities.h"
#include "Prefab.h"
#include "Shader.h"

#include "stb_image.h"

#include <glm/glm.hpp>
#include <json/single_include/nlohmann/json.hpp>

namespace
{
	// JSON constants.
	const std::string PROPERTY_NAME{ "name" };
	const std::string PROPERTY_LAYOUT{ "layout" };
	const std::string PROPERTY_TILES{ "tiles" };
	const std::string PROPERTY_BGTEXTURE{ "bgTexture" };
	const std::string PROPERTY_BGM{ "bgm" };
	const std::string PROPERTY_SHADER{ "shader" };
	const std::string PROPERTY_LAYERS{ "layers" };
	const std::string PROPERTY_DEPTH{ "depth" };
	const std::string PROPERTY_SPRITESHEET{ "spriteSheet" };
	const std::string PROPERTY_TYPE{ "type" };
	const std::string PROPERTY_X{ "x" };
	const std::string PROPERTY_Y{ "y" };
	const std::string PROPERTY_ENTITIES{ "entities" };
}

Room::Room(Prefab *prefab, AssetLoader *assetLoader)
{
	if (prefab != nullptr)
	{
		parseJson(prefab->getJson(), assetLoader);
	}
}

glm::ivec2 Room::getSize() const
{
	return m_tiles->getSize();
}

const Room::TileType &Room::getTileType(glm::ivec2 tileCoord) const
{
	// Flip the y-coordinate.
	glm::ivec2 size{ getSize() };
	tileCoord.y = size.y - tileCoord.y - 1;

	int index{ tileCoord.x + size.x * tileCoord.y };
	index = glm::clamp(index, 0, size.x * size.y - 1);
	return m_layout[index];
}

const glm::ivec2 Room::getTileCoord(glm::vec2 pos) const
{
	return { floor(pos.x / TILE_SIZE), floor(pos.y / TILE_SIZE) };
}

const glm::vec2 Room::getTilePos(glm::ivec2 tileCoord) const
{
	return { tileCoord.x * TILE_SIZE + TILE_SIZE / 2.f, 
		tileCoord.y * TILE_SIZE + TILE_SIZE / 2.f };
}

Texture *Room::getTileSprites() const
{
	return m_tiles.get();
}

bool Room::isSlope(TileType type)
{
	return type == TILE_SLOPE_RIGHT_LOWER || type == TILE_SLOPE_RIGHT_UPPER ||
		type == TILE_SLOPE_LEFT_UPPER || type == TILE_SLOPE_LEFT_LOWER;
}

void Room::parseJson(const nlohmann::json &json, AssetLoader *assetLoader)
{
	try
	{
		// Read the contents into json.
		if (JSONUtilities::hasEntry(PROPERTY_NAME, json))
		{
			m_name = json.at(PROPERTY_NAME).get<std::string>();
		}

		if (JSONUtilities::hasEntry(PROPERTY_LAYOUT, json))
		{
			m_layout = json.at(PROPERTY_LAYOUT).get<std::vector<TileType>>();
		}

		if (JSONUtilities::hasEntry(PROPERTY_TILES, json))
		{
			std::string tilesName = { json.at(PROPERTY_TILES).get<std::string>() };
			if (!tilesName.empty())
			{
				m_tiles = assetLoader->load<Texture>(tilesName);
			}
		}

		if (JSONUtilities::hasEntry(PROPERTY_BGTEXTURE, json))
		{
			std::string bgTextureName = { json.at(PROPERTY_BGTEXTURE).get<std::string>() };
			if (!bgTextureName.empty())
			{
				m_bgTexture = assetLoader->load<Texture>(bgTextureName);
			}
		}

		if (JSONUtilities::hasEntry(PROPERTY_SHADER, json))
		{
			std::string shaderName{ json.at(PROPERTY_SHADER).get<std::string>() };
			if (!shaderName.empty())
			{
				m_shader = assetLoader->load<Shader>(shaderName);
			}
		}

		if (JSONUtilities::hasEntry(PROPERTY_LAYERS, json))
		{
			const nlohmann::json &layersJson{ json.at(PROPERTY_LAYERS) };

			if (!layersJson.is_array())
			{
				std::cout << "Room::parseJson: '" <<
					PROPERTY_LAYERS << "' property is not an array" << std::endl;
				return;
			}

			// TODO: Parse each layer.
			for (const auto &layer : layersJson)
			{
				if (JSONUtilities::hasEntry(PROPERTY_DEPTH, layer))
				{
					int depth{ layer.at(PROPERTY_DEPTH).get<int>() };
				}

				if (JSONUtilities::hasEntry(PROPERTY_SPRITESHEET, layer))
				{
					std::string spriteSheetName{ layer.at(PROPERTY_SPRITESHEET).get<std::string>() };
				}

				if (JSONUtilities::hasEntry(PROPERTY_TYPE, layer))
				{
					std::string type{ layer.at(PROPERTY_TYPE).get<std::string>() };
				}

				if (JSONUtilities::hasEntry(PROPERTY_X, layer))
				{
					int x{ layer.at(PROPERTY_X).get<int>() };
				}

				if (JSONUtilities::hasEntry(PROPERTY_Y, layer))
				{
					int y{ layer.at(PROPERTY_Y).get<int>() };
				}
			}
		}

		if (JSONUtilities::hasEntry(PROPERTY_ENTITIES, json))
		{
			const nlohmann::json &entitiesJson{ json.at(PROPERTY_ENTITIES) };

			if (!entitiesJson.is_array())
			{
				std::cout << "Room::parseJson: '" <<
					PROPERTY_ENTITIES << "' property is not an array" << std::endl;
				return;
			}

			// TODO: Parse each entity.
			for (const auto &entity : entitiesJson)
			{
				if (JSONUtilities::hasEntry(PROPERTY_NAME, entity))
				{
					std::string prefabName{ entity.at(PROPERTY_NAME).get<std::string>() };
				}

				if (JSONUtilities::hasEntry(PROPERTY_TYPE, entity))
				{
					std::string type{ entity.at(PROPERTY_TYPE).get<std::string>() };
				}

				if (JSONUtilities::hasEntry(PROPERTY_X, entity))
				{
					int x{ entity.at(PROPERTY_X).get<int>() };
				}

				if (JSONUtilities::hasEntry(PROPERTY_Y, entity))
				{
					int y{ entity.at(PROPERTY_Y).get<int>() };
				}
			}
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		std::cout << "Room::parseJson: " << e.what() << std::endl;
	}
}