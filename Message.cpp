#include "Message.hpp"

template<typename T>
size_t get_net_size(const T& data) {
	static_assert(std::is_standard_layout<T>::value, "Can not decide net size of non-standard_layout type");
	return sizeof(data);
}

template<typename T>
size_t get_net_size(const std::vector<T>& data) {
	size_t sz = sizeof(uint16_t);

	for (const T& d : data) {
		sz += get_net_size(d);
	}

	return sz;
}

template<>
size_t get_net_size<std::string>(const std::string& data) {
	return sizeof(uint16_t) + sizeof(std::string::value_type) * data.size();
}
