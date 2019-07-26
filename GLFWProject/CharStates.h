#pragma once
#ifndef CharStates_H
#define CharStates_H

#include <string>

namespace CharState
{
	extern const std::string IDLE;
	extern const std::string RUN;
	extern const std::string JUMP_ASCEND;
	extern const std::string JUMP_PEAK;
	extern const std::string JUMP_DESCEND;
	extern const std::string JUMP_LAND;
	extern const std::string RUN_START;
	extern const std::string RUN_STOP;
	extern const std::string ALERT;
	extern const std::string TURN;
	extern const std::string CROUCH;
	extern const std::string CROUCH_STOP;
	extern const std::string ATTACK;
	extern const std::string ATTACK_AIR;
	extern const std::string ATTACK_CROUCH;
	extern const std::string HURT;
	extern const std::string HURT_AIR;
	extern const std::string FALLEN;
}

#endif