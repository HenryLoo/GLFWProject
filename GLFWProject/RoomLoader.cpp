#include "RoomLoader.h"

#include "Texture.h"

#include "stb_image.h"

namespace
{
	// Define rgb values for each tile type.
	// These will be used in the layout texture.
	unsigned char RGB_SPACE[3]{ 255, 255, 255 };
	unsigned char RGB_WALL[3]{ 0, 0, 0 };
	unsigned char RGB_GHOST[3]{ 0, 0, 255 };
	unsigned char RGB_SLOPE_LEFT_1[3]{ 0, 160, 0 };
	unsigned char RGB_SLOPE_LEFT_2[3]{ 0, 255, 0 };
	unsigned char RGB_SLOPE_RIGHT_1[3]{ 255, 0, 0 };
	unsigned char RGB_SLOPE_RIGHT_2[3]{ 160, 0, 0 };

	const int NUM_STREAMS_REQUIRED{ 2 };
}

RoomLoader::RoomLoader()
{
	m_numStreamsRequired = NUM_STREAMS_REQUIRED;
}

std::shared_ptr<IAssetType> RoomLoader::loadFromStream(
	const std::vector<IDataStream::Result> &streams, 
	const std::string &name, int flag)
{
	const IDataStream::Result &layoutResult{ streams[0] };
	const IDataStream::Result &tilesResult{ streams[1] };

	// Create the texture for the room's tiles.
	GLuint textureId;
	int width, height, numChannels;
	if (loadValues(tilesResult, textureId, width, height, numChannels))
	{
		std::shared_ptr<Texture> tiles{ 
			std::make_shared<Texture>(textureId, width, height, numChannels) };

		// Get the layout of the room.
		std::vector<Room::TileType> layout;
		loadLayout(layoutResult, layout);

		// Create the room asset from the layout and tiles texture.
		std::shared_ptr<Room> room{
			std::make_shared<Room>(layout, tiles) };

		if (room != nullptr)
		{
			std::cout << "RoomLoader::loadFromStream: Loaded '" << name << "'\n" << std::endl;
			return room;
		}
	}

	return nullptr;
}

void RoomLoader::loadLayout(const IDataStream::Result &streamedData, 
	std::vector<Room::TileType> &output)
{
	stbi_set_flip_vertically_on_load(true);

	// Load the texture from the buffer.
	int length{ streamedData.length };
	char *buffer{ new char[length] };
	streamedData.stream->read(buffer, length);
	glm::ivec2 layoutSize;
	int numChannels;
	stbi_uc *data{ stbi_load_from_memory((stbi_uc *)buffer, length, 
		&layoutSize.x, &layoutSize.y, &numChannels, 0) };

	for (int i = 0; i < layoutSize.y; ++i)
	{
		for (int j = 0; j < layoutSize.x; ++j)
		{

			const stbi_uc *pixel{ data + (4 * (i * layoutSize.x + j)) };
			if (isTileType(pixel, RGB_SPACE))
			{
				output.push_back(Room::TILE_SPACE);
				continue;
			}
			else if (isTileType(pixel, RGB_WALL))
			{
				output.push_back(Room::TILE_WALL);
				continue;
			}
			else if (isTileType(pixel, RGB_GHOST))
			{
				output.push_back(Room::TILE_GHOST);
				continue;
			}
			else if (isTileType(pixel, RGB_SLOPE_LEFT_1))
			{
				output.push_back(Room::TILE_SLOPE_LEFT_UPPER);
				continue;
			}

			else if (isTileType(pixel, RGB_SLOPE_LEFT_2))
			{
				output.push_back(Room::TILE_SLOPE_LEFT_LOWER);
				continue;
			}
			else if (isTileType(pixel, RGB_SLOPE_RIGHT_1))
			{
				output.push_back(Room::TILE_SLOPE_RIGHT_LOWER);
				continue;
			}
			else if (isTileType(pixel, RGB_SLOPE_RIGHT_2))
			{
				output.push_back(Room::TILE_SLOPE_RIGHT_UPPER);
				continue;
			}
			// Placeholder tile, in case nothing matches.
			else
			{
				output.push_back(Room::TILE_SPACE);
				continue;
			}
		}
	}

	// Free the image memory after we're done with it.
	stbi_image_free(data);
	delete buffer;
}

bool RoomLoader::isTileType(const unsigned char *pixel, unsigned char rgb[3])
{
	stbi_uc r{ pixel[0] };
	stbi_uc g{ pixel[1] };
	stbi_uc b{ pixel[2] };

	return rgb[0] == r && rgb[1] == g && rgb[2] == b;
}