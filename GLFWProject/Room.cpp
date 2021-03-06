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

Room::Room(Prefab *prefab, AssetLoader *assetLoader, SpriteRenderer *sRenderer)
{
	if (prefab != nullptr)
	{
		parseJson(prefab->getJson(), assetLoader, sRenderer);
	}
}

glm::ivec2 Room::getSize() const
{
	return m_data.size;
}

int Room::getTileIndex(glm::ivec2 tileCoord) const
{
	// Flip the y-coordinate.
	glm::ivec2 size{ getSize() };
	tileCoord.x = glm::clamp(tileCoord.x, 0, size.x - 1);
	tileCoord.y = glm::clamp(tileCoord.y, 0, size.y - 1);
	tileCoord.y = size.y - tileCoord.y - 1;

	return tileCoord.x + size.x * tileCoord.y;
}

RoomData::TileType Room::getTileType(glm::ivec2 tileCoord) const
{
	int index{ getTileIndex(tileCoord) };
	return static_cast<RoomData::TileType>(m_data.layout[index]);
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

std::shared_ptr<Texture> Room::getTileSprites() const
{
	return m_tilesTexture;
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
	return type == RoomData::TILE_SLOPE_L_LOWER || 
		type == RoomData::TILE_SLOPE_L_UPPER ||
		type == RoomData::TILE_SLOPE_R_UPPER ||
		type == RoomData::TILE_SLOPE_R_LOWER;
}

void Room::parseJson(const nlohmann::json &json, AssetLoader *assetLoader,
	SpriteRenderer *sRenderer)
{
	try
	{
		// Read the contents into json.
		if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_NAME, json))
		{
			m_data.name = json.at(RoomConstants::PROPERTY_NAME).get<std::string>();
		}

		if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_SIZE, json))
		{
			const nlohmann::json &thisSize = json.at(RoomConstants::PROPERTY_SIZE);

			if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_X, thisSize))
			{
				m_data.size.x = thisSize.at(RoomConstants::PROPERTY_X).get<int>();
			}

			if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_Y, thisSize))
			{
				m_data.size.y = thisSize.at(RoomConstants::PROPERTY_Y).get<int>();
			}
		}

		if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_LAYOUT, json))
		{
			m_data.layout = json.at(RoomConstants::PROPERTY_LAYOUT)
				.get<std::vector<int>>();
		}

		if (JSONUtilities::hasEntry(RoomConstants::PROPERTY_TILES, json))
		{
			m_data.tiles = json.at(RoomConstants::PROPERTY_TILES)
				.get<std::vector<int>>();
			m_tilesTexture = createTilesTexture(sRenderer, m_data.size, m_data.tiles);
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

void Room::setTiles(const std::vector<int> &tileConfig)
{
	m_data.tiles = tileConfig;
}

void Room::setTilesTexture(std::shared_ptr<Texture> tilesTexture)
{
	m_tilesTexture = tilesTexture;
}

void Room::setLayout(const std::vector<int> &layout)
{
	m_data.layout = layout;
}

void Room::setSize(glm::ivec2 size)
{
	m_data.size = size;
}

void Room::setLayers(const std::vector<RoomData::Layer> &layers)
{
	m_data.layers = layers;
}

void Room::setLayerSpriteSheet(int id, AssetLoader *assetLoader, 
	const std::string &spriteSheetName)
{
	if (spriteSheetName.empty())
		return;

	id = glm::clamp(id, 0, static_cast<int>(m_data.layers.size() - 1));
	m_data.layers[id].spriteSheetName = spriteSheetName;
	m_layerSpriteSheets[id] = assetLoader->load<SpriteSheet>(spriteSheetName);
}

void Room::setLayerType(int id, const std::string &type)
{
	id = glm::clamp(id, 0, static_cast<int>(m_data.layers.size() - 1));
	m_data.layers[id].type = type;
}

void Room::setLayerPos(int id, glm::vec2 pos)
{
	id = glm::clamp(id, 0, static_cast<int>(m_data.layers.size() - 1));
	m_data.layers[id].pos.x = pos.x;
	m_data.layers[id].pos.y = pos.y;
}

void Room::setLayerDepth(int id, float depth)
{
	id = glm::clamp(id, 0, static_cast<int>(m_data.layers.size() - 1));
	m_data.layers[id].pos.z = depth;
}

void Room::addLayer()
{
	m_data.layers.push_back({});
	m_layerSpriteSheets.push_back(nullptr);
}

void Room::deleteLayer(int id)
{
	id = glm::clamp(id, 0, static_cast<int>(m_data.layers.size() - 1));
	m_data.layers.erase(m_data.layers.begin() + id);
	m_layerSpriteSheets.erase(m_layerSpriteSheets.begin() + id);
}

std::shared_ptr<Texture> Room::createTilesTexture(SpriteRenderer *sRenderer,
	glm::ivec2 roomSize, const std::vector<int> &tileConfig)
{
	if (sRenderer == nullptr)
		return nullptr;
	glm::ivec2 tilesetSize{ sRenderer->getTilesetSize() };
	glm::ivec2 numTiles{ tilesetSize / Room::TILE_SIZE };

	const int NUM_CHANNELS{ 4 };
	GLubyte *data{ new GLubyte[roomSize.x * roomSize.y * NUM_CHANNELS] };
	for (int i = 0; i < roomSize.y; ++i)
	{
		for (int j = 0; j < roomSize.x; ++j)
		{
			GLubyte r{ 255 }, g{ 255 }, b{ 255 }, a{ 255 };
			int layoutIndex{ roomSize.x * i + j };
			if (tileConfig[layoutIndex] != RoomData::TILE_SPACE)
			{
				int tileIndex{ tileConfig[layoutIndex] - 1 };
				r = (GLubyte)(tileIndex) % numTiles.x;
				g = (GLubyte)floor(tileIndex / numTiles.x);
				b = 0;
			}

			// Image is vertically flipped.
			int firstIndex{ NUM_CHANNELS * (roomSize.x * roomSize.y - roomSize.x * (i + 1) + j) };
			data[firstIndex] = r;
			data[firstIndex + 1] = g;
			data[firstIndex + 2] = b;
			data[firstIndex + 3] = a;
		}
	}

	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, roomSize.x, roomSize.y, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);

	auto texture{ std::make_shared<Texture>(textureId, roomSize.x, roomSize.y, NUM_CHANNELS) };

	delete[] data;

	return texture;
}