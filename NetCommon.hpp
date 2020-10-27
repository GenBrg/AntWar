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

enum class ChannelId : uint8_t
{
	CONTROL_CHANNEL = 0,
	LEFT_SIDE,
	RIGHT_SIDE
};

inline const std::string kMagicHeader { "ATWR" };
