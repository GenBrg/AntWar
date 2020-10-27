#pragma once

#include "NetCommon.hpp"
#include "Ant.hpp"
#include "Structure.hpp"
#include "Scene.hpp"

#include <deque>

class Player 
{
public:
	inline static constexpr size_t kStructureNum { 4 };

	Player(ChannelId side);

	void Update(float elapsed);
	void Draw();

	// #ifdef SERVER
	
	void UpdatePlayerStub();
	void UpgradeStructureImpl(MessageReader<RpcId, 4>& reader);
	void SellStructureImpl(MessageReader<RpcId, 4>& reader);
	void BuyStructureImpl(MessageReader<RpcId, 4>& reader);
	void SummonAntImpl(MessageReader<RpcId, 4>& reader);
	// #endif

	// #ifdef CLIENT
	void UpdatePlayerImpl(MessageReader<RpcId, 4>& reader);
	void UpgradeStructureStub(uint8_t structure_id);
	void SellStructureStub(uint8_t structure_id);
	void BuyStructureStub(Structure::Type structure_type, uint8_t structure_id);
	void SummonAntStub(Ant::Type type);
	// #endif

	int GetResearchLevel() const;
	int GetAntPrice() const;
	int GetManpowerGainRate() const;
	
	bool IsLeftSide() const { return side_ == ChannelId::LEFT_SIDE; }

	void Draw(Scene& scene);

	void SendRpcs(std::vector<uint8_t>& send_buffer);
	MessageDispatcher<RpcId, 4>& GetMessageDispatcher() { return message_dispatcher; }

private: 
	ChannelId side_;
	int hp_ { 1000 };
	int man_power_ { 0 };
	Structure structures_[kStructureNum];
	std::deque<Ant> ants_;

	std::vector<Message<RpcId, 4>> queued_rpcs_;
	MessageDispatcher<RpcId, 4> message_dispatcher { kMagicHeader };
};


