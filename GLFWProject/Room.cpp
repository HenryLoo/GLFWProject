#include "Room.h"
#include "Texture.h"

#include "stb_image.h"

#include <glm/glm.hpp>

namespace
{
	const std::string TEXTURE_PATH{ "textures/" };
	const std::string TILES_EXT{ "_tiles.png" };
	const std::string LAYOUT_EXT{ "_layout.png" };

	// Define rgb values for each tile type.
	// These will be used in the layout texture.
	unsigned char RGB_SPACE[3]{ 255, 255, 255 };
	unsigned char RGB_WALL[3]{ 0, 0, 0 };
	unsigned char RGB_GHOST[3]{ 0, 0, 255 };
	unsigned char RGB_SLOPE_LEFT_1[3]{ 0, 160, 0 };
	unsigned char RGB_SLOPE_LEFT_2[3]{ 0, 255, 0 };
	unsigned char RGB_SLOPE_RIGHT_1[3]{ 255, 0, 0 };
	unsigned char RGB_SLOPE_RIGHT_2[3]{ 160, 0, 0 };
}

Room::Room(const std::string &roomName)
{
	// TODO: replace this with asset loader stuff later.
	GLint width, height, numChan;

	// OpenGL expects 0.0 of the y-axis to be on the bottom, but images have it
	// at the top. So we need to flip the image.
	stbi_set_flip_vertically_on_load(true);

	// Load the texture from the file.
	const std::string tilePath{ TEXTURE_PATH + roomName + TILES_EXT };
	stbi_uc *tileData{ stbi_load(tilePath.c_str(), &width, &height, &numChan, 0) };

	GLuint textureId;
	bool success{ false };
	if (tileData != nullptr)
	{
		// Generate the texture for OpenGL and store its id.
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// Set texture parameters for the bound texture.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Create the texture from the loaded file.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, tileData);
	}

	// Free the image memory after we're done with it.
	stbi_image_free(tileData);

	m_tileSprites = std::make_unique<Texture>(textureId, width, height, numChan);

	// TODO END;

	stbi_set_flip_vertically_on_load(true);

	// Load the texture from the file.
	const std::string path{ TEXTURE_PATH + roomName + LAYOUT_EXT };
	glm::ivec2 layoutSize;
	int numChannels;
	stbi_uc *data{ stbi_load(path.c_str(), &layoutSize.x, &layoutSize.y, &numChannels, 0) };

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
			else if (isTileType(pixel, RGB_SLOPE_LEFT_1))
			{
				m_tileTypes.push_back(TILE_SLOPE_LEFT_UPPER);
				continue;
			}

			else if (isTileType(pixel, RGB_SLOPE_LEFT_2))
			{
				m_tileTypes.push_back(TILE_SLOPE_LEFT_LOWER);
				continue;
			}
			else if (isTileType(pixel, RGB_SLOPE_RIGHT_1))
			{
				m_tileTypes.push_back(TILE_SLOPE_RIGHT_LOWER);
				continue;
			}
			else if (isTileType(pixel, RGB_SLOPE_RIGHT_2))
			{
				m_tileTypes.push_back(TILE_SLOPE_RIGHT_UPPER);
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

bool Room::isSlope(TileType type)
{
	return type == TILE_SLOPE_RIGHT_LOWER || type == TILE_SLOPE_RIGHT_UPPER ||
		type == TILE_SLOPE_LEFT_UPPER || type == TILE_SLOPE_LEFT_LOWER;
}