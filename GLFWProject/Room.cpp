#include "Room.h"

#include "Texture.h"
#include "AssetLoader.h"
#include "JSONUtilities.h"
#include "Prefab.h"
#include "Shader.h"
#include "SpriteRenderer.h"

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

Texture *Room::getBgTexture() const
{
	return m_bgTexture.get();
}

Shader *Room::getShader() const
{
	return m_shader.get();
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

			for (const auto &layer : layersJson)
			{
				Layer thisLayer;

				if (JSONUtilities::hasEntry(PROPERTY_DEPTH, layer))
				{
					thisLayer.pos.z = layer.at(PROPERTY_DEPTH).get<float>();
				}

				if (JSONUtilities::hasEntry(PROPERTY_SPRITESHEET, layer))
				{
					std::string spriteSheetName{ layer.at(PROPERTY_SPRITESHEET).get<std::string>() };
					if (!spriteSheetName.empty())
					{
						thisLayer.spriteSheet = assetLoader->load<SpriteSheet>(spriteSheetName);
					}

					if (thisLayer.spriteSheet == nullptr)
					{
						continue;
					}
				}

				if (JSONUtilities::hasEntry(PROPERTY_TYPE, layer))
				{
					thisLayer.type = layer.at(PROPERTY_TYPE).get<std::string>();
					if (thisLayer.type.empty())
						continue;
				}

				// Get the tile coords x, y and convert them to world positions.
				glm::ivec2 tileCoord{ 0 };
				if (JSONUtilities::hasEntry(PROPERTY_X, layer))
				{
					tileCoord.x = layer.at(PROPERTY_X).get<int>();
				}

				if (JSONUtilities::hasEntry(PROPERTY_Y, layer))
				{
					tileCoord.y = layer.at(PROPERTY_Y).get<int>();
				}

				glm::vec2 tilePos{ getTilePos(tileCoord) };
				glm::ivec2 clipSize{ 0 };
				SpriteSheet::SpriteSet set;
				if (thisLayer.spriteSheet->getSprite(thisLayer.type, set))
				{
					clipSize = set.clips[0].clipSize;
				}

				tilePos.y = tilePos.y - (TILE_SIZE - clipSize.y) / 2.f;
				thisLayer.pos.x = tilePos.x;
				thisLayer.pos.y = tilePos.y;

				m_layers.push_back(thisLayer);
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
				EntityListing entityList;

				if (JSONUtilities::hasEntry(PROPERTY_NAME, entity))
				{
					entityList.prefabName = entity.at(PROPERTY_NAME).get<std::string>();
				}

				if (JSONUtilities::hasEntry(PROPERTY_TYPE, entity))
				{
					entityList.type = entity.at(PROPERTY_TYPE).get<std::string>();
				}


				// Get the tile coords x, y and convert them to world positions.
				glm::ivec2 tileCoord{ 0 };
				if (JSONUtilities::hasEntry(PROPERTY_X, entity))
				{
					tileCoord.x = entity.at(PROPERTY_X).get<int>();
				}

				if (JSONUtilities::hasEntry(PROPERTY_Y, entity))
				{
					tileCoord.y = entity.at(PROPERTY_Y).get<int>();
				}

				glm::vec2 tilePos{ getTilePos(tileCoord) };
				entityList.pos.x = tilePos.x;
				entityList.pos.y = tilePos.y;

				m_entities.push_back(entityList);
			}
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		std::cout << "Room::parseJson: " << e.what() << std::endl;
	}
}

void Room::updateLayers(SpriteRenderer *sRenderer) const
{
	for (const Layer &layer : m_layers)
	{
		sRenderer->addSprite(layer);
	}
}

const std::vector<Room::EntityListing> &Room::getEntities() const
{
	return m_entities;
}