#pragma once
#ifndef RoomData_H
#define RoomData_h

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace RoomData
{
	enum TileType
	{
		TILE_SPACE,
		TILE_WALL,
		TILE_GHOST,
		TILE_SLOPE_L_LOWER,
		TILE_SLOPE_L_UPPER,
		TILE_SLOPE_R_UPPER,
		TILE_SLOPE_R_LOWER,
		NUM_TILE_TYPES
	};

	struct Layer
	{
		// The sprite sheet to take this layer from.
		std::string spriteSheetName;

		// The sprite type on the sprite sheet.
		std::string type;

		// Defines the origin of the layer.
		glm::vec3 pos;
	};

	struct Data
	{
		std::string name;

		glm::ivec2 size;

		// Hold all the tile in this room.
		// Tiles are stored in row-major order, bottom-up.
		std::vector<TileType> layout;
		std::vector<int> tiles;

		std::string bgTextureName;
		std::string musicName;
		std::string shaderName;
		std::vector<Layer> layers;
	};
}

#endif