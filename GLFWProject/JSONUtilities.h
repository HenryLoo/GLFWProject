#pragma once
#ifndef JSONUtilities_H
#define JSONUtilities_H

#include <json/include/nlohmann/json_fwd.hpp>

#include <string>

namespace RoomData
{
	struct Data;
}

namespace JSONUtilities
{
	// Check if a JSON element has an entry with a given key.
	bool hasEntry(const std::string &key, const nlohmann::json &json);

	// Create a JSON object with a room's details.
	void roomToJson(RoomData::Data room, nlohmann::json &json);
}

#endif