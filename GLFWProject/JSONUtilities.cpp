#include "JSONUtilities.h"

#include "RoomData.h"
#include "RoomConstants.h"

#include <json/single_include/nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

namespace JSONUtilities
{
	bool hasEntry(const std::string &key, const json &json)
	{
		bool hasEntry{ json.count(key) == 1 };
		return hasEntry;
	}

	void roomToJson(RoomData::Data room, nlohmann::json &json)
	{
		json[RoomConstants::PROPERTY_NAME] = room.name;
		json[RoomConstants::PROPERTY_LAYOUT] = room.layout;
		json[RoomConstants::PROPERTY_TILES] = room.tilesName;
		json[RoomConstants::PROPERTY_BGTEXTURE] = room.bgTextureName;
		json[RoomConstants::PROPERTY_MUSIC] = room.musicName;
		json[RoomConstants::PROPERTY_SHADER] = room.shaderName;
		std::vector<nlohmann::json> layers;
		for (const RoomData::Layer &data : room.layers)
		{
			layers.push_back({ 
				{ RoomConstants::PROPERTY_DEPTH, data.pos.z },
				{ RoomConstants::PROPERTY_SPRITESHEET, data.spriteSheetName },
				{ RoomConstants::PROPERTY_TYPE, data.type },
				{ RoomConstants::PROPERTY_X, data.pos.x },
				{ RoomConstants::PROPERTY_Y, data.pos.y }
			});
		}
		json[RoomConstants::PROPERTY_LAYERS] = layers;
	}
}