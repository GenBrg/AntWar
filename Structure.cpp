#include "Structure.hpp"

Message<RpcId, 4>& operator << (Message<RpcId, 4>& msg, const Structure& structure)
{
	msg << structure.level_ << structure.type_;
	return msg;
}

MessageReader<RpcId, 4>& operator >> (MessageReader<RpcId, 4>& reader, Structure& structure)
{
	reader >> structure.level_ >> structure.type_;
	return reader;
}

int Structure::GetBuyPrice(Type type)
{
	switch (type)
	{
		case Type::NONE:
			return 0;
		break;
		case Type::RESEARCH_CENTER:
			return 2000;
		break;
		case Type::STORAGE:
			return 1000;
		break;
		default:
			return 0;
	}
}

int Structure::GetSellPrice()
{
	int base_price = GetBuyPrice(type_);

	return base_price * (level_ * level_ + level_) / 40;
}

int Structure::GetUpgradePrice()
{
	int base_price = GetBuyPrice(type_);

	return base_price * (level_ * 0.1f + 1);
}
