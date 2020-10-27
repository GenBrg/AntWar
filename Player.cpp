#include "Player.hpp"

#include <chrono>

Player::Player(ChannelId side) :
side_(side)
{
	message_dispatcher.RegisterMessageCallback(RpcId::BUILD_STRUCTURE, [this](MessageReader<RpcId, 4> reader){BuyStructureImpl(reader);});
	message_dispatcher.RegisterMessageCallback(RpcId::UPDATE_PLAYER, [this](MessageReader<RpcId, 4> reader){UpdatePlayerImpl(reader);});
	message_dispatcher.RegisterMessageCallback(RpcId::UPGRADE_STRUCTURE, [this](MessageReader<RpcId, 4> reader){UpgradeStructureImpl(reader);});
	message_dispatcher.RegisterMessageCallback(RpcId::SELL_STRUCTURE, [this](MessageReader<RpcId, 4> reader){SellStructureImpl(reader);});
	message_dispatcher.RegisterMessageCallback(RpcId::SUMMON_ANT, [this](MessageReader<RpcId, 4> reader){SummonAntImpl(reader);});
}

void Player::Update(float elapsed)
{
	static float gain_manpower_cooldown { 0.0f };
	constexpr float kGainManpowerInterval { 5.0f };

	gain_manpower_cooldown -= elapsed;
	if (gain_manpower_cooldown <= 0.0f) {
		man_power_ += GetManpowerGainRate();
		gain_manpower_cooldown += kGainManpowerInterval;
	}
}

void Player::UpdatePlayerStub()
{
	queued_rpcs_.emplace_back(kMagicHeader, RpcId::UPDATE_PLAYER);
	auto& msg = queued_rpcs_.back();

	msg << hp_ << man_power_;
	msg << structures_;
	msg << std::vector<Ant>(ants_.begin(), ants_.end());
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

	int buy_price = Structure::GetBuyPrice(type);
	if (man_power_ < buy_price) {
		return;
	}

	man_power_ -= buy_price;
	structures_[structure_id] = Structure(type);
}

void Player::SummonAntImpl(MessageReader<RpcId, 4>& reader)
{
	static std::chrono::high_resolution_clock::time_point last_summon_time;
	constexpr std::chrono::milliseconds kSummonCoolDown{ 3000 };

	auto now = std::chrono::high_resolution_clock::now();

	if (now - last_summon_time < kSummonCoolDown) {
		return;
	}

	int ant_price = GetAntPrice();

	if (man_power_ < ant_price) {
		return;
	}

	man_power_ -= ant_price;

	last_summon_time = now;

	Ant::Type type;

	reader >> type;
	ants_.emplace_back(type, GetResearchLevel(), IsLeftSide());
}

void Player::UpdatePlayerImpl(MessageReader<RpcId, 4>& reader)
{
	Message<RpcId, 4> msg(kMagicHeader, RpcId::UPDATE_PLAYER);

	reader >> hp_ >> man_power_;
	reader >> structures_;
	std::vector<Ant> ants;
	reader >> ants;

	ants_ = std::deque<Ant>(ants.begin(), ants.end());
}

void Player::UpgradeStructureStub(uint8_t structure_id)
{
	queued_rpcs_.emplace_back(kMagicHeader, RpcId::UPGRADE_STRUCTURE);
	auto& msg = queued_rpcs_.back();

	msg << structure_id;
}

void Player::SellStructureStub(uint8_t structure_id)
{
	queued_rpcs_.emplace_back(kMagicHeader, RpcId::SELL_STRUCTURE);
	auto& msg = queued_rpcs_.back();

	msg << structure_id;
}

void Player::BuyStructureStub(Structure::Type structure_type, uint8_t structure_id)
{
	queued_rpcs_.emplace_back(kMagicHeader, RpcId::BUILD_STRUCTURE);
	auto& msg = queued_rpcs_.back();

	msg << structure_type << structure_id;
}

void Player::SummonAntStub(Ant::Type type)
{
	queued_rpcs_.emplace_back(kMagicHeader, RpcId::SUMMON_ANT);
	auto& msg = queued_rpcs_.back();

	msg << type;
}

int Player::GetResearchLevel() const
{
	return 1;
}

int Player::GetAntPrice() const
{
	return 500;
}

int Player::GetManpowerGainRate() const
{
	return 1000;
}

void Player::Draw(Scene& scene)
{
	for (const auto& structure : structures_) {
		structure.Draw(scene);
	}

	for (const auto& ant : ants_) {
		ant.Draw(scene);
	}
}

void Player::SendRpcs(std::vector<uint8_t>& send_buffer)
{
	Message<ChannelId, 4> msg(kMagicHeader, side_);

	for (const auto& rpc : queued_rpcs_) {
		msg << rpc;
	}

	msg.Send(send_buffer);
}
