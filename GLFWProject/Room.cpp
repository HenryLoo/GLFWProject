#include "Room.h"
#include "Texture.h"

#include "stb_image.h"

#include <glm/glm.hpp>

namespace
{
	const std::string TILES_EXT{ "_tiles.png" };
	const std::string LAYOUT_EXT{ "_layout.png" };

	// Define rgb values for each tile type.
	// These will be used in the layout texture.
	unsigned char RGB_SPACE[3]{ 255, 255, 255 };
	unsigned char RGB_WALL[3]{ 0, 0, 0 };
	unsigned char RGB_GHOST[3]{ 0, 0, 255 };
}

Room::Room(const std::string &roomName)
{
	m_tileSprites = std::make_unique<Texture>(roomName + TILES_EXT);

	stbi_set_flip_vertically_on_load(true);

	// Load the texture from the file.
	const std::string path{ Texture::TEXTURE_PATH + roomName + LAYOUT_EXT };
	glm::ivec2 layoutSize;
	int numChannels;
	stbi_uc *data{ stbi_load(path.c_str(), &layoutSize.x, &layoutSize.y, &numChannels, 0) };

	// TODO: replace hard-coded walls.
	for (int i = 0; i < layoutSize.y; ++i)
	{
		for (int j = 0; j < layoutSize.x; ++j)
		{

			const stbi_uc *pixel{ data + (4 * (i * layoutSize.x + j)) };
			if (isTileType(pixel, RGB_SPACE))
			{
				m_tileTypes.push_back(TILE_SPACE);
				continue;
			}
			else if (isTileType(pixel, RGB_WALL))
			{
				m_tileTypes.push_back(TILE_WALL);
				continue;
			}
			else if (isTileType(pixel, RGB_GHOST))
			{
				m_tileTypes.push_back(TILE_GHOST);
				continue;
			}
			// Placeholder tile, in case nothing matches.
			else
			{
				m_tileTypes.push_back(TILE_SPACE);
				continue;
			}
		}
	}

	// Free the image memory after we're done with it.
	stbi_image_free(data);
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

bool Room::isTileType(const unsigned char *pixel, unsigned char rgb[3])
{
	stbi_uc r{ pixel[0] };
	stbi_uc g{ pixel[1] };
	stbi_uc b{ pixel[2] };

	return rgb[0] == r && rgb[1] == g && rgb[2] == b;
}