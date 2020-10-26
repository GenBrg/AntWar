#pragma once

#include "NetCommon.hpp"
#include "Message.hpp"


class Structure {
public:
	enum class Type : uint8_t
	{
		NONE = 0,
		RESEARCH_CENTER,
		STORAGE
	};

	friend Message<RpcId, 4>& operator << (Message<RpcId, 4>& msg, const Structure& structure);
	friend MessageReader<RpcId, 4>& operator >> (MessageReader<RpcId, 4>& reader, Structure& structure);

	static int GetBuyPrice(Type type);
	int GetSellPrice();
	int GetUpgradePrice();

	void Upgrade() { ++level_; }
	Type GetType() const { return type_; }

	Structure(Type type): type_(type) {}
	Structure() {}

private:
	int level_ { 1 };
	Type type_ { Type::NONE };
};
