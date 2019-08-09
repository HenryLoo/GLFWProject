#pragma once
#ifndef Room_H
#define Room_H

#include "Texture.h"
#include "IAssetType.h"

#include <glm/glm.hpp>

#include <vector>
#include <memory>

class SpriteRenderer;

class Room : public IAssetType
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

	Room(const std::vector<Room::TileType> &layout,
		std::shared_ptr<Texture> tiles);
	virtual ~Room();

	// Get the size of the room.
	glm::ivec2 getSize() const;

	// Get the tile at a given x, y, in tile coordinates.
	const Room::TileType &getTileType(glm::ivec2 tileCoord) const;

	// Get the tile coordinates corresponding to given world coordinates.
	const glm::ivec2 getTileCoord(glm::vec2 pos) const;

	// Get the tile's world position, given its tile coordinates.
	const glm::vec2 getTilePos(glm::ivec2 tileCoord) const;

	// Get this room's tile sprites.
	Texture *getTileSprites() const;

	// Check if a given tile type is a slope.
	static bool isSlope(TileType type);

	// The size of each tile, in pixels.
	const static int TILE_SIZE{ 16 };
	const static int SLOPE_HEIGHT{ 8 };

private:
	// Hold all the tiles in this room.
	// Tiles are stored in row-major order, bottom-up.
	std::vector<TileType> m_layout;

	// Hold each tile's index on the sprite sheet.
	std::shared_ptr<Texture> m_tiles;
};

#endif