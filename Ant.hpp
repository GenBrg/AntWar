#pragma once

#include "Message.hpp"

class Ant {
public:
	enum class Type : uint8_t {
		FIGHTER = 0
	};

	Ant(Type type, int level);

private:
	Type type_;
	int hp_;
	int level_;
	int attack_;
	int defense_;

	float position_;
};
		