#include "Room.h"

Room::Room(glm::vec2 size, const std::vector<TileType> &tileTypes,
	const std::vector<unsigned int> &tileSprites) :
	m_size(size), m_tileTypes(tileTypes), m_tileSprites(tileSprites)
{

}

Room::~Room()
{

}

const glm::ivec2 &Room::getSize() const
{
	return m_size;
}

const TileType &Room::getTileType(int x, int y) const
{
	int index{ x + m_size.x * y };
	glm::clamp(index, 0, m_size.x * m_size.y);
	return m_tileTypes[index];
}

const std::vector<unsigned int> &Room::getTileSprites() const
{
	return m_tileSprites;
}