#pragma once

#include "NetCommon.hpp"
#include "Ant.hpp"
#include "Structure.hpp"

#include <deque>

class Player 
{
public:
	inline static constexpr size_t kStructureNum { 4 };

	Player(MessageDispatcher<RpcId, 4>& message_dispatcher);

	void Update(float elapsed);
	void Draw();

	// #ifdef SERVER
	
	void UpdatePlayerStub(std::vector<uint8_t>& send_buffer);
	void UpgradeStructureImpl(MessageReader<RpcId, 4>& reader);
	void SellStructureImpl(MessageReader<RpcId, 4>& reader);
	void BuyStructureImpl(MessageReader<RpcId, 4>& reader);
	void SummonAntImpl(MessageReader<RpcId, 4>& reader);
	// #endif

	// #ifdef CLIENT
	void UpdatePlayerImpl(MessageReader<RpcId, 4>& reader);
	void UpgradeStructureStub(std::vector<uint8_t>& send_buffer, uint8_t structure_id);
	void SellStructureStub(std::vector<uint8_t>& send_buffer, uint8_t structure_id);
	void BuyStructureStub(std::vector<uint8_t>& send_buffer, Structure::Type structure_type, uint8_t structure_id);
	void SummonAntStub(std::vector<uint8_t>& send_buffer);
	// #endif

private: 
	bool left_side;
	int hp_ { 1000 };
	int man_power_ { 0 };
	Structure structures_[kStructureNum];
	std::deque<Ant> ants_;
};


