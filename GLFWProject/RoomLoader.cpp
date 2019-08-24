#include "RoomLoader.h"

#include "Texture.h"
#include "JSONUtilities.h"

#include "stb_image.h"

#include <json/single_include/nlohmann/json.hpp>

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

	const int NUM_STREAMS_REQUIRED{ 3 };

	// JSON constants.
	const std::string PROPERTY_NAME{ "name" };
	const std::string PROPERTY_BGTEXTURE{ "bgTexture" };
	const std::string PROPERTY_SHADER{ "shader" };
	const std::string PROPERTY_LAYERS{ "layers" };
	const std::string PROPERTY_DEPTH{ "depth" };
	const std::string PROPERTY_SPRITESHEET{ "spriteSheet" };
	const std::string PROPERTY_TYPE{ "type" };
	const std::string PROPERTY_X{ "x" };
	const std::string PROPERTY_Y{ "y" };
	const std::string PROPERTY_ENTITIES{ "entities" };
}

RoomLoader::RoomLoader()
{
	m_numStreamsRequired = NUM_STREAMS_REQUIRED;
}

std::shared_ptr<IAssetType> RoomLoader::loadFromStream(
	const std::vector<IDataStream::Result> &streams, 
	const std::string &name, int flag)
{
	const IDataStream::Result &jsonResult{ streams[0] };
	const IDataStream::Result &layoutResult{ streams[1] };
	const IDataStream::Result &tilesResult{ streams[2] };

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
		std::shared_ptr<Room> room{ loadRoom(jsonResult, layout, tiles) };

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

std::shared_ptr<Room> RoomLoader::loadRoom(const IDataStream::Result &streamedData,
	std::vector<Room::TileType> &layout, std::shared_ptr<Texture> tiles)
{
	try
	{
		// Read the contents into json.
		nlohmann::json j;
		*(streamedData.stream) >> j;

		std::string roomName;
		if (JSONUtilities::hasEntry(PROPERTY_NAME, j))
		{
			roomName = j.at(PROPERTY_NAME).get<std::string>();
		}

		if (JSONUtilities::hasEntry(PROPERTY_BGTEXTURE, j))
		{
			std::string bgTextureName{ j.at(PROPERTY_BGTEXTURE).get<std::string>() };

			// Create the background texture.

		}

		if (JSONUtilities::hasEntry(PROPERTY_SHADER, j))
		{
			std::string shaderName{ j.at(PROPERTY_SHADER).get<std::string>() };

			// Load the shader.

		}

		if (JSONUtilities::hasEntry(PROPERTY_LAYERS, j))
		{
			const nlohmann::json &layersJson{ j.at(PROPERTY_LAYERS) };

			if (!layersJson.is_array())
			{
				std::cout << "RoomLoader::loadRoom: '" <<
					PROPERTY_LAYERS << "' property is not an array" << std::endl;
				return nullptr;
			}

			// Parse each layer.
			for (const auto &layer : layersJson)
			{
				if (JSONUtilities::hasEntry(PROPERTY_DEPTH, layer))
				{
					int depth{ layer.at(PROPERTY_DEPTH).get<int>() };
				}

				if (JSONUtilities::hasEntry(PROPERTY_SPRITESHEET, layer))
				{
					std::string spriteSheetName{ layer.at(PROPERTY_SPRITESHEET).get<std::string>() };
				}

				if (JSONUtilities::hasEntry(PROPERTY_TYPE, layer))
				{
					std::string type{ layer.at(PROPERTY_TYPE).get<std::string>() };
				}

				if (JSONUtilities::hasEntry(PROPERTY_X, layer))
				{
					int x{ layer.at(PROPERTY_X).get<int>() };
				}

				if (JSONUtilities::hasEntry(PROPERTY_Y, layer))
				{
					int y{ layer.at(PROPERTY_Y).get<int>() };
				}
			}
		}

		if (JSONUtilities::hasEntry(PROPERTY_ENTITIES, j))
		{
			const nlohmann::json &entitiesJson{ j.at(PROPERTY_ENTITIES) };

			if (!entitiesJson.is_array())
			{
				std::cout << "RoomLoader::loadRoom: '" <<
					PROPERTY_ENTITIES << "' property is not an array" << std::endl;
				return nullptr;
			}

			// Parse each entity.
			for (const auto &entity : entitiesJson)
			{
				if (JSONUtilities::hasEntry(PROPERTY_NAME, entity))
				{
					std::string prefabName{ entity.at(PROPERTY_NAME).get<std::string>() };
				}

				if (JSONUtilities::hasEntry(PROPERTY_TYPE, entity))
				{
					std::string type{ entity.at(PROPERTY_TYPE).get<std::string>() };
				}

				if (JSONUtilities::hasEntry(PROPERTY_X, entity))
				{
					int x{ entity.at(PROPERTY_X).get<int>() };
				}

				if (JSONUtilities::hasEntry(PROPERTY_Y, entity))
				{
					int y{ entity.at(PROPERTY_Y).get<int>() };
				}
			}
		}
	}
	catch (const nlohmann::json::exception &e)
	{
		std::cout << "RoomLoader::loadRoom: " << e.what() << std::endl;
	}

	// Load the room with the given data.
	return std::make_shared<Room>(layout, tiles);
}