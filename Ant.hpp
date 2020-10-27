#pragma once

#include "Message.hpp"
#include "NetCommon.hpp"
#include "Scene.hpp"

class Ant {
public:
	enum class Type : uint8_t {
		FIGHTER = 0
	};

	Ant() {}
	Ant(Type type, int level, bool left_side);
	void Draw(Scene& scene) const;

	friend Message<RpcId, 4>& operator << (Message<RpcId, 4>& msg, const Ant& structure);
	friend MessageReader<RpcId, 4>& operator >> (MessageReader<RpcId, 4>& reader, Ant& structure);
	
	friend size_t get_net_size(const Ant& data);

private:
	Type type_;
	int hp_;
	int level_;
	int attack_;
	int defense_;
	bool left_side_;
	float position_;

	Scene::Transform transform_;
	Scene::Drawable drawable_;
};
