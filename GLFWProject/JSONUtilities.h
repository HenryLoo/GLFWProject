#pragma once
#ifndef JSONUtilities_H
#define JSONUtilities_H

#include <string>
#include <json/include/nlohmann/json_fwd.hpp>

namespace JSONUtilities
{
	bool hasEntry(const std::string &key, const nlohmann::json &json);
}

#endif