#pragma once
#ifndef Script_H
#define Script_H

#include "IAssetType.h"

#include <sol/sol.hpp>

#include <string>

class Script : public IAssetType
{
public:
	Script(const std::string &script);
	virtual ~Script();

	// Execute the script and return the result.
	sol::protected_function_result execute(sol::state &lua) const;

private:
	std::string m_script;
};

#endif