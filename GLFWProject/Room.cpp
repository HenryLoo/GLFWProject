#include "Room.h"
#include "Texture.h"

#include <glm/glm.hpp>

namespace
{
	const std::string TILES_EXT{ "_tiles.png" };
	const std::string LAYOUT_EXT{ "_layout.png" };
}

Room::Room(const std::string &roomName)
{
	m_tileSprites = std::make_unique<Texture>(roomName + TILES_EXT);

	// TODO: replace hard-coded walls.
	for (int i = 0; i < 26 * 16; ++i)
	{
		if (i < 26 * 2)
		{
			m_tileTypes.push_back(TILE_WALL);
			continue;
		}

		m_tileTypes.push_back(TILE_SPACE);
	}
}

Room::~Room()
{

}

glm::ivec2 Room::getSize() const
{
	return m_tileSprites->getSize();
}

const TileType &Room::getTileType(glm::ivec2 tileCoord) const
{
	glm::ivec2 size{ getSize() };
	int index{ tileCoord.x + size.x * tileCoord.y };
	index = glm::clamp(index, 0, size.x * size.y - 1);
	return m_tileTypes[index];
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
	return m_tileSprites.get();
}