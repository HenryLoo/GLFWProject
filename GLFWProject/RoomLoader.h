#pragma once
#ifndef RoomLoader_H
#define RoomLoader_H

#include "TextureLoader.h"
#include "Room.h"

class RoomLoader : public TextureLoader
{
public:
	RoomLoader();

private:
	virtual std::shared_ptr<IAssetType> loadFromStream(
		const std::vector<IDataStream::Result> &streams,
		const std::string &name, int flag);

	// Load the layout of tile types for the room and output it into
	// the output vector param.
	void loadLayout(const IDataStream::Result &streamedData,
		std::vector<Room::TileType> &output);

	// Check if a pixel matches the given rgb values.
	// This is used to determine the tile type of a tile from a layout texture.
	bool isTileType(const unsigned char *pixel, unsigned char rgb[3]);

	// Load the room details from the json file and then instantiate the room
	// with these, along with the provided layout and tiles.
	// Return nullptr if room could not be loaded.
	std::shared_ptr<Room> loadRoom(const IDataStream::Result &streamedData,
		std::vector<Room::TileType> &layout, std::shared_ptr<Texture> tiles);
};

#endif