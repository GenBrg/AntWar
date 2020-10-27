#include "Message.hpp"

size_t get_net_size(const std::string& data) {
	return sizeof(uint16_t) + sizeof(std::string::value_type) * data.size();
}
