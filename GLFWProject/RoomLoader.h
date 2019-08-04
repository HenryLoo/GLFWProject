#pragma once
#ifndef RoomLoader_H
#define RoomLoader_H

#include "TextureLoader.h"
#include "Room.h"

class RoomLoader : public TextureLoader
{
public:
	std::shared_ptr<IAssetType> load(
		const std::vector<IDataStream::Result> &streams, 
		const std::string &name);

	// Get the number of streams required for this loader.
	virtual int getNumStreamsRequired() const;

private:
	// Load the layout of tile types for the room and output it into
	// the output vector param.
	void loadLayout(const IDataStream::Result &streamedData,
		std::vector<Room::TileType> &output);

	// Check if a pixel matches the given rgb values.
	// This is used to determine the tile type of a tile from a layout texture.
	bool isTileType(const unsigned char *pixel, unsigned char rgb[3]);
};

#endif