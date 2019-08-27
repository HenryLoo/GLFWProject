#pragma once
#ifndef Room_H
#define Room_H

#include "Texture.h"

#include <glm/glm.hpp>
#include <json/include/nlohmann/json_fwd.hpp>

#include <vector>
#include <memory>

class SpriteRenderer;
class Prefab;
class AssetLoader;
class Shader;
class SpriteSheet;

class Room
{
public:
	enum TileType
	{
		TILE_SPACE,
		TILE_WALL,
		TILE_SLOPE_LEFT_UPPER,
		TILE_SLOPE_LEFT_LOWER,
		TILE_SLOPE_RIGHT_UPPER,
		TILE_SLOPE_RIGHT_LOWER,
		TILE_GHOST
	};

	// Defines background/foreground layers in rooms.
	struct Layer
	{
		// The sprite sheet to take this layer from.
		std::shared_ptr<SpriteSheet> spriteSheet;

		// The sprite type on the sprite sheet.
		std::string type;

		// Defines the origin of the layer.
		glm::vec3 pos;
	};

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

	Room(Prefab *prefab, AssetLoader *assetLoader);

	// Get the size of the room.
	glm::ivec2 getSize() const;

	// Get the tile at a given x, y, in tile coordinates.
	const TileType &getTileType(glm::ivec2 tileCoord) const;

	// Get the tile coordinates corresponding to given world coordinates.
	const glm::ivec2 getTileCoord(glm::vec2 pos) const;

	// Get the tile's world position, given its tile coordinates.
	const glm::vec2 getTilePos(glm::ivec2 tileCoord) const;

	// Get this room's tile sprites.
	Texture *getTileSprites() const;

	// Get this room's background texture.
	Texture *getBgTexture() const;

	// Get this room's shader.
	Shader *getShader() const;

	// Check if a given tile type is a slope.
	static bool isSlope(TileType type);

	// Add the layer sprite data to the sprite renderer.
	void updateLayers(SpriteRenderer *sRenderer) const;

	// Get the list of entities to create.
	const std::vector<EntityListing> &getEntities() const;

	// The size of each tile, in pixels.
	const static int TILE_SIZE{ 16 };
	const static int SLOPE_HEIGHT{ 8 };

private:
	// Load the room details from the json file.
	void parseJson(const nlohmann::json &json, AssetLoader *assetLoader);

	// The room's name.
	std::string m_name;

	// Hold all the tiles in this room.
	// Tiles are stored in row-major order, bottom-up.
	std::vector<TileType> m_layout;

	// Hold each tile's index on the sprite sheet.
	std::shared_ptr<Texture> m_tiles;

	// The background texture.
	std::shared_ptr<Texture> m_bgTexture;

	// The room's shader. This is applied in the post-processing framebuffer.
	std::shared_ptr<Shader> m_shader;

	// Hold the room's background and foreground layers.
	std::vector<Layer> m_layers;

	// Hold the list of entities to create.
	std::vector<EntityListing> m_entities;
};

#endif