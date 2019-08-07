#include "Prefab.h"

Prefab::Prefab(nlohmann::json &json) :
	m_json(json)
{

}

const nlohmann::json &Prefab::getJson()
{
	return m_json;
}