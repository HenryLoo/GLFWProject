#pragma once
#ifndef Prefab_H
#define Prefab_H

#include "IAssetType.h"

#include <json/single_include/nlohmann/json.hpp>

class Prefab : public IAssetType
{
public:
	Prefab(nlohmann::json &json);

	const nlohmann::json &getJson();

private:
	virtual void cleanup() {};

	nlohmann::json m_json;
};

#endif