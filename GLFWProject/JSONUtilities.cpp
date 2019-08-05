#include "JSONUtilities.h"

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
}