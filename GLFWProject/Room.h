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
	Room(const std::vector<TileType> &tileTypes, 
		const std::string &spritePath);
	//Room(glm::vec2 size, const std::vector<TileType> &tileTypes,
	//	const std::vector<unsigned int> &tileSprites);
	~Room();

	// Get the size of the room.
	glm::ivec2 getSize() const;

	// Get the tile at a given x, y, in tile coordinates.
	const TileType &getTileType(int x, int y) const;

	// Get this room's tile sprites.
	//const std::vector<unsigned int> &getTileSprites() const;
	Texture *getTileSprites() const;

	// The size of each tile, in pixels.
	//const static int TILE_SIZE{ 32 };

private:
	// The width and height of the room, in tiles.
	//glm::ivec2 m_size;

	// Hold all the tiles in this room.
	// Tiles are stored in row-major order, bottom-up.
	std::vector<TileType> m_tileTypes;

	// Hold each tile's index on the sprite sheet.
	//std::vector<unsigned int> m_tileSprites;
	std::unique_ptr<Texture> m_tileSprites;
};

#endif