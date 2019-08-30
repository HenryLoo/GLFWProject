#include "Room.h"

#include "Texture.h"
#include "AssetLoader.h"
#include "JSONUtilities.h"
#include "Prefab.h"
#include "Music.h"
#include "Shader.h"
#include "SpriteRenderer.h"
#include "RoomConstants.h"

#include "stb_image.h"

#include <glm/glm.hpp>
#include <json/single_include/nlohmann/json.hpp>

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

const RoomData::TileType &Room::getTileType(glm::ivec2 tileCoord) const
{
	// Flip the y-coordinate.
	glm::ivec2 size{ getSize() };
	tileCoord.x = glm::clamp(tileCoord.x, 0, size.x - 1);
	tileCoord.y = glm::clamp(tileCoord.y, 0, size.y - 1);
	tileCoord.y = size.y - tileCoord.y - 1;

	int index{ tileCoord.x + size.x * tileCoord.y };
	return m_data.layout[index];
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

Music *Room::getMusic() const
{
	return m_music.get();
}

const RoomData::Data &Room::getRoomData() const
{
	return m_data;
}

bool Room::isSlope(RoomData::TileType type)
{
	return type == RoomData::TILE_SLOPE_RIGHT_LOWER || 
		type == RoomData::TILE_SLOPE_RIGHT_UPPER ||
		type == RoomData::TILE_SLOPE_LEFT_UPPER ||
		type == RoomData::TILE_SLOPE_LEFT_LOWER;
}

void Room::parseJson(const nlohmann::json &json, AssetLoader *assetLoader)
{
	try
	{
		// Read the contents into json.
		if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_NAME, json))
		{
			m_data.name = json.at(RoomConstants::PROPERTY_NAME).get<std::string>();
		}

		if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_LAYOUT, json))
		{
			m_data.layout = json.at(RoomConstants::PROPERTY_LAYOUT)
				.get<std::vector<RoomData::TileType>>();
		}

		if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_TILES, json))
		{
			m_data.tilesName = json.at(RoomConstants::PROPERTY_TILES)
				.get<std::string>();
			if (!m_data.tilesName.empty())
			{
				m_tiles = assetLoader->load<Texture>(m_data.tilesName);
			}
		}

		if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_BGTEXTURE, json))
		{
			m_data.bgTextureName = json.at(RoomConstants::PROPERTY_BGTEXTURE)
				.get<std::string>();
			if (!m_data.bgTextureName.empty())
			{
				m_bgTexture = assetLoader->load<Texture>(m_data.bgTextureName);
			}
		}

		if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_MUSIC, json))
		{
			m_data.musicName = json.at(RoomConstants::PROPERTY_MUSIC)
				.get<std::string>();
			if (!m_data.musicName.empty())
			{
				m_music = assetLoader->load<Music>(m_data.musicName);
			}
		}

		if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_SHADER, json))
		{
			m_data.shaderName = json.at(RoomConstants::PROPERTY_SHADER)
				.get<std::string>();
			if (!m_data.shaderName.empty())
			{
				m_shader = assetLoader->load<Shader>(m_data.shaderName);
			}
		}

		if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_LAYERS, json))
		{
			const nlohmann::json &layersJson{ json.at(RoomConstants::PROPERTY_LAYERS) };

			if (!layersJson.is_array())
			{
				std::cout << "Room::parseJson: '" <<
					RoomConstants::PROPERTY_LAYERS << 
					"' property is not an array" << std::endl;
				return;
			}

			for (const auto &layer : layersJson)
			{
				RoomData::Layer thisLayer;

				if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_DEPTH, layer))
				{
					thisLayer.pos.z = layer.at(RoomConstants::PROPERTY_DEPTH)
						.get<float>();
				}

				std::shared_ptr<SpriteSheet> spriteSheet{ nullptr };
				if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_SPRITESHEET, layer))
				{
					thisLayer.spriteSheetName = layer.at(RoomConstants::PROPERTY_SPRITESHEET)
						.get<std::string>();
					if (!thisLayer.spriteSheetName.empty())
					{
						spriteSheet = assetLoader->load<SpriteSheet>(thisLayer.spriteSheetName);
					}

					if (spriteSheet == nullptr)
					{
						continue;
					}
				}

				if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_TYPE, layer))
				{
					thisLayer.type = layer.at(RoomConstants::PROPERTY_TYPE)
						.get<std::string>();
					if (thisLayer.type.empty())
						continue;
				}

				// Get the tile coords x, y and convert them to world positions.
				//glm::ivec2 tileCoord{ 0 };
				if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_X, layer))
				{
					thisLayer.pos.x = layer.at(RoomConstants::PROPERTY_X).get<float>();
				}

				if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_Y, layer))
				{
					thisLayer.pos.y = layer.at(RoomConstants::PROPERTY_Y).get<float>();
				}

				//glm::vec2 tilePos{ getTilePos(tileCoord) };
				//glm::ivec2 clipSize{ 0 };
				//SpriteSheet::SpriteSet set;
				//if (spriteSheet->getSprite(thisLayer.type, set))
				//{
				//	clipSize = set.clips[0].clipSize;
				//}

				//tilePos.y = tilePos.y - (TILE_SIZE - clipSize.y) / 2.f;
				//thisLayer.pos.x = tilePos.x;
				//thisLayer.pos.y = tilePos.y;

				m_data.layers.push_back(thisLayer);
				m_layerSpriteSheets.push_back(spriteSheet);
			}
		}

		if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_ENTITIES, json))
		{
			const nlohmann::json &entitiesJson{ json.at(RoomConstants::PROPERTY_ENTITIES) };

			if (!entitiesJson.is_array())
			{
				std::cout << "Room::parseJson: '" <<
					RoomConstants::PROPERTY_ENTITIES <<
					"' property is not an array" << std::endl;
				return;
			}

			// TODO: Parse each entity.
			for (const auto &entity : entitiesJson)
			{
				EntityListing entityList;

				if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_NAME, entity))
				{
					entityList.prefabName = entity.at(RoomConstants::PROPERTY_NAME)
						.get<std::string>();
				}

				if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_TYPE, entity))
				{
					entityList.type = entity.at(RoomConstants::PROPERTY_TYPE)
						.get<std::string>();
				}


				// Get the tile coords x, y and convert them to world positions.
				glm::ivec2 tileCoord{ 0 };
				if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_X, entity))
				{
					tileCoord.x = entity.at(RoomConstants::PROPERTY_X).get<int>();
				}

				if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_Y, entity))
				{
					tileCoord.y = entity.at(RoomConstants::PROPERTY_Y).get<int>();
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
	for (int i = 0; i < m_layerSpriteSheets.size(); ++i)
	{
		sRenderer->addSprite(m_data.layers[i], m_layerSpriteSheets[i]);
	}
}

const std::vector<Room::EntityListing> &Room::getEntities() const
{
	return m_entities;
}