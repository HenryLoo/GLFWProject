#include "Room.h"
#include "Texture.h"

Room::Room(const std::vector<TileType> &tileTypes,
	const std::string &spritePath) :
	m_tileTypes(tileTypes)
{
	m_tileSprites = std::make_unique<Texture>(spritePath);
}

//Room::Room(glm::vec2 size, const std::vector<TileType> &tileTypes,
//	const std::vector<unsigned int> &tileSprites) :
//	m_size(size), m_tileTypes(tileTypes), m_tileSprites(tileSprites)
//{
//
//}

Room::~Room()
{

}

glm::ivec2 Room::getSize() const
{
	//return m_size;
	return m_tileSprites->getSize();
}

const TileType &Room::getTileType(int x, int y) const
{
	glm::ivec2 size{ getSize() };
	int index{ x + size.x * y };
	glm::clamp(index, 0, size.x * size.y);
	return m_tileTypes[index];
}

Texture *Room::getTileSprites() const
{
	return m_tileSprites.get();
}

//const std::vector<unsigned int> &Room::getTileSprites() const
//{
//	return m_tileSprites;
//}