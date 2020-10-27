#include "Ant.hpp"

#include "LitColorTextureProgram.hpp"
#include "PlayMode.hpp"

Ant::Ant(Type type, int level, bool left_side) :
type_(type),
level_(level),
left_side_(left_side)
{
	hp_ = 100 + level * 10;
	attack_ = 10 + level_ * 2;
	defense_ = 5 + level_ * 1;
	position_ = (left_side) ? -5.0f : 5.0f;
}

Message<RpcId, 4>& operator << (Message<RpcId, 4>& msg, const Ant& ant)
{
	msg << ant.type_ << ant.hp_ << ant.level_ << ant.attack_ << ant.defense_ << ant.left_side_ << ant.position_;
	return msg;
}

MessageReader<RpcId, 4>& operator >> (MessageReader<RpcId, 4>& reader, Ant& ant)
{
	reader >> ant.type_ >> ant.hp_ >> ant.level_ >> ant.attack_ >> ant.defense_ >> ant.left_side_ >> ant.position_;

	std::string to_load = (ant.left_side_) ? "BlueAnt" : "GreenAnt";
	ant.drawable_.pipeline = lit_color_texture_program_pipeline;
	ant.drawable_.pipeline.mesh = &ant_war_meshes->lookup(to_load);
	ant.drawable_.transform = &ant.transform_;

	ant.transform_.position = glm::vec3(0.0f, ant.position_, 0.0f);

	return reader;
}

void Ant::Draw(Scene& scene) const
{
	scene.dynamic_drawables.emplace_back(drawable_);
}

size_t get_net_size(const Ant& data)
{
	return sizeof(data.type_) + sizeof(data.hp_) + sizeof(data.level_) + sizeof(data.attack_) + sizeof(data.defense_) + sizeof(data.left_side_) + sizeof(data.position_);
}
