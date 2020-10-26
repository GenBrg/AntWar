#include "Player.hpp"

Player::Player(MessageDispatcher<RpcId, 4>& message_dispatcher)
{
	message_dispatcher.RegisterMessageCallback(RpcId::BUILD_STRUCTURE, [this](MessageReader<RpcId, 4> reader){BuyStructureImpl(reader);});
	message_dispatcher.RegisterMessageCallback(RpcId::UPDATE_PLAYER, [this](MessageReader<RpcId, 4> reader){UpdatePlayerImpl(reader);});
	message_dispatcher.RegisterMessageCallback(RpcId::UPGRADE_STRUCTURE, [this](MessageReader<RpcId, 4> reader){UpgradeStructureImpl(reader);});
	message_dispatcher.RegisterMessageCallback(RpcId::SELL_STRUCTURE, [this](MessageReader<RpcId, 4> reader){SellStructureImpl(reader);});
	message_dispatcher.RegisterMessageCallback(RpcId::SUMMON_ANT, [this](MessageReader<RpcId, 4> reader){SummonAntImpl(reader);});
}

void Player::Update(float elapsed)
{

}

void Player::UpdatePlayerStub(std::vector<uint8_t>& send_buffer)
{
	Message<RpcId, 4> msg(kMagicHeader, RpcId::UPDATE_PLAYER);

	msg << hp_ << man_power_;
	msg << structures_;
	msg << std::vector<Ant>(ants_.begin(), ants_.end());

	msg.Send(send_buffer);
}

void Player::UpgradeStructureImpl(MessageReader<RpcId, 4>& reader)
{
	uint8_t structure_id;
	reader >> structure_id;

	// Validate
	if (structure_id < 0 || structure_id >= kStructureNum) {
		return;
	}

	if (structures_[structure_id].GetType() == Structure::Type::NONE) {
		return;
	}

	int upgrade_price = structures_[structure_id].GetUpgradePrice();
	if (man_power_ < upgrade_price) {
		return;
	}

	man_power_ -= upgrade_price;
	structures_[structure_id].Upgrade();
}

void Player::SellStructureImpl(MessageReader<RpcId, 4>& reader)
{
	uint8_t structure_id;
	reader >> structure_id;

	// Validate
	if (structure_id < 0 || structure_id >= kStructureNum) {
		return;
	}

	if (structures_[structure_id].GetType() == Structure::Type::NONE) {
		return;
	}

	man_power_ += structures_[structure_id].GetSellPrice();
	structures_[structure_id] = Structure(Structure::Type::NONE);
}

void Player::BuyStructureImpl(MessageReader<RpcId, 4>& reader)
{
	Structure::Type type;
	uint8_t structure_id;
	reader >> type >> structure_id;

	// Validate
	if (structure_id < 0 || structure_id >= kStructureNum) {
		return;
	}

	if (structures_[structure_id].GetType() != Structure::Type::NONE) {
		return;
	}

	int buy_price = Structure::GetBuyPrice();
	if (man_power_ < buy_price) {
		return;
	}

	man_power_ -= buy_price;
	structures_[structure_id] = Structure(type);
}

void Player::SummonAntImpl(MessageReader<RpcId, 4>& reader)
{

}
