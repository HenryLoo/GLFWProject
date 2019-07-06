#include "CharStates.h"

namespace PlayerState
{
	const std::string IDLE{ "idle" };
	const std::string RUN{ "run" };
	const std::string JUMP_ASCEND{ "jump_ascend" };
	const std::string JUMP_PEAK{ "jump_peak" };
	const std::string JUMP_DESCEND{ "jump_descend" };
	const std::string JUMP_LAND{ "jump_land" };
	const std::string RUN_START{ "run_start" };
	const std::string RUN_STOP{ "run_stop" };
	const std::string ALERT{ "alert" };
	const std::string TURN{ "turn" };
	const std::string CROUCH{ "crouch" };
	const std::string CROUCH_STOP{ "crouch_stop" };
	const std::string ATTACK{ "attack" };
	const std::string ATTACK_AIR{ "attack_air" };
	const std::string ATTACK_CROUCH{ "attack_crouch" };
}