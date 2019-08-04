#include "Room.h"
#include "Texture.h"

#include "stb_image.h"

#include <glm/glm.hpp>

Room::Room(const std::vector<Room::TileType> &layout,
	std::shared_ptr<Texture> tiles) :
	m_layout(layout), m_tiles(tiles)
{

}

Room::~Room()
{

}

glm::ivec2 Room::getSize() const
{
	return m_tiles->getSize();
}

const Room::TileType &Room::getTileType(glm::ivec2 tileCoord) const
{
	glm::ivec2 size{ getSize() };
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