#pragma once
#ifndef CharStates_H
#define CharStates_H

#include <string>

namespace PlayerState
{
	extern const std::string IDLE;
	extern const std::string RUN;
	extern const std::string JUMP_ASCEND;
	extern const std::string JUMP_PEAK;
	extern const std::string JUMP_DESCEND;
	extern const std::string RUN_START;
	extern const std::string RUN_STOP;
	extern const std::string ALERT;
	extern const std::string TURN;
}

#endif