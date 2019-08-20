#include "Script.h"

#include <iostream>

Script::Script(const std::string &script) : m_script(script)
{

}

Script::~Script()
{

}

bool Script::execute(sol::state &lua) const
{
	auto result{ lua.script(m_script, [](lua_State *, sol::protected_function_result pfr) {
		return pfr;
	}) };

	bool success{ result.valid() };
	if (!success)
		std::cout << "Script::execute: invalid script:" << std::endl << m_script << std::endl;

	return success;
}