#pragma once
#ifndef Room_H
#define Room_H

#include "Texture.h"

#include <glm/glm.hpp>

#include <vector>
#include <memory>

class SpriteRenderer;

enum TileType
{
	TILE_SPACE,
	TILE_WALL,
	TILE_SLOPE_LEFT,
	TILE_SLOPE_RIGHT,
	TILE_GHOST
};

class Room
{
public:
	Room(const std::string &roomName);
	~Room();

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

	// The size of each tile, in pixels.
	const static int TILE_SIZE{ 16 };
	const static int SLOPE_HEIGHT{ 8 };

private:
	// Check if a pixel matches the given rgb values.
	// This is used to determine the tile type of a tile from a layout texture.
	bool isTileType(const unsigned char *pixel, unsigned char rgb[3]);

	// Hold all the tiles in this room.
	// Tiles are stored in row-major order, bottom-up.
	std::vector<TileType> m_tileTypes;

	// Hold each tile's index on the sprite sheet.
	std::unique_ptr<Texture> m_tileSprites;
};

#endif