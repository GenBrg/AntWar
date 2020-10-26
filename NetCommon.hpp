#pragma once

#include "Message.hpp"

enum class RpcId : uint8_t
{
	UPDATE_PLAYER = 0,
	UPGRADE_STRUCTURE,
	BUILD_STRUCTURE,
	SELL_STRUCTURE,
	SUMMON_ANT
};

inline constexpr std::string kMagicHeader { "ATWR" };
