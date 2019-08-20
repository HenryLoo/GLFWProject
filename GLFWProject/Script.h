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

	// Execute the script.
	// Return true if there were no errors, else false.
	bool execute(sol::state &lua) const;

private:
	std::string m_script;
};

#endif