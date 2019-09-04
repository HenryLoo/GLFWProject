#pragma once
#ifndef Room_H
#define Room_H

#include "Texture.h"
#include "RoomData.h"

#include <glm/glm.hpp>
#include <json/include/nlohmann/json_fwd.hpp>

#include <vector>
#include <memory>

class SpriteRenderer;
class Prefab;
class AssetLoader;
class Shader;
class SpriteSheet;
class Music;

class Room
{
public:
	// Defines an entity to be created in the room.
	struct EntityListing
	{
		// The name of the entity's prefab.
		std::string prefabName;

		// The type of entity to create.
		std::string type;

		// Defines the position in the room to spawn the enemy in.
		glm::vec2 pos;
	};

	Room(Prefab *prefab, AssetLoader *assetLoader, SpriteRenderer *sRenderer);

	// Get the size of the room.
	glm::ivec2 getSize() const;

	// Get the tile's index at a given x, y, in tile coordinates.
	int getTileIndex(glm::ivec2 tileCoord) const;

	// Get the tile at a given x, y, in tile coordinates.
	const RoomData::TileType &getTileType(glm::ivec2 tileCoord) const;

	// Get the tile coordinates corresponding to given world coordinates.
	const glm::ivec2 getTileCoord(glm::vec2 pos) const;

	// Get the tile's world position, given its tile coordinates.
	const glm::vec2 getTilePos(glm::ivec2 tileCoord) const;

	// Get this room's tile sprites.
	std::shared_ptr<Texture> getTileSprites() const;

	// Get this room's background texture.
	Texture *getBgTexture() const;

	// Get this room's shader.
	Shader *getShader() const;

	// Get this room's music.
	Music *getMusic() const;

	// Get the room's data.
	const RoomData::Data &getRoomData() const;

	// Check if a given tile type is a slope.
	static bool isSlope(RoomData::TileType type);

	// Add the layer sprite data to the sprite renderer.
	void updateLayers(SpriteRenderer *sRenderer) const;

	// Get the list of entities to create.
	const std::vector<EntityListing> &getEntities() const;

	// The size of each tile, in pixels.
	const static int TILE_SIZE{ 16 };
	const static int SLOPE_HEIGHT{ 8 };

	// Set the tile configuration.
	void setTiles(const std::vector<int> &tileConfig);

	// Set the tiles texture.
	void setTilesTexture(std::shared_ptr<Texture> tiles);

	// Set the layout.
	void setLayout(const std::vector<RoomData::TileType> &layout);

	// Set the layers.
	void setLayers(const std::vector<RoomData::Layer> &layers);

	// Layer setters.
	void setLayerSpriteSheet(int id, AssetLoader *assetLoader, 
		const std::string &spriteSheetName);
	void setLayerType(int id, const std::string &type);
	void setLayerPos(int id, glm::vec2 pos);
	void setLayerDepth(int id, float depth);

	// Add a layer.
	void addLayer();

	// Delete a layer.
	void deleteLayer(int id);

	// Create a tile texture from a given tile configuration.
	static std::shared_ptr<Texture> createTilesTexture(SpriteRenderer *sRenderer,
		glm::ivec2 roomSize,  const std::vector<int> &tileConfig);

private:
	// Load the room details from the json file.
	void parseJson(const nlohmann::json &json, AssetLoader *assetLoader, 
		SpriteRenderer *sRenderer);

	// The room's data.
	RoomData::Data m_data;

	// Hold each tile's index on the sprite sheet.
	std::shared_ptr<Texture> m_tilesTexture;

	// The background texture.
	std::shared_ptr<Texture> m_bgTexture;

	// The room's shader. This is applied in the post-processing framebuffer.
	std::shared_ptr<Shader> m_shader;

	// The background music to play when this room is the current room.
	std::shared_ptr<Music> m_music;

	// Hold the spritesheets that are used by the room's background and 
	// foreground layers.
	std::vector<std::shared_ptr<SpriteSheet>> m_layerSpriteSheets;

	// Hold the list of entities to create.
	std::vector<EntityListing> m_entities;
};

#endif