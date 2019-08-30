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
		TILE_SLOPE_LEFT_UPPER,
		TILE_SLOPE_LEFT_LOWER,
		TILE_SLOPE_RIGHT_UPPER,
		TILE_SLOPE_RIGHT_LOWER,
		TILE_GHOST
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

		// Hold all the tiles in this room.
		// Tiles are stored in row-major order, bottom-up.
		std::vector<TileType> layout;

		std::string tilesName;
		std::string bgTextureName;
		std::string musicName;
		std::string shaderName;
		std::vector<Layer> layers;
	};
}

#endif