// Some ideas of class Message are from 
// https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Videos/Networking/Parts1%262/net_message.h
//
// Credit to javidx9 for his recent video on game networking in C++: https://www.youtube.com/watch?v=2hNdkYInj4g&ab_channel=javidx9

#pragma once

#include <cstdint>
#include <vector>
#include <functional>
#include <unordered_map>
#include <string>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <cassert>

template <typename T, size_t MagicHeaderSize>
class Message;

template <typename T, size_t MagicHeaderSize>
class MessageReader;

template <typename T, size_t MagicHeaderSize>
class MessageDispatcher;

template<typename T>
size_t get_net_size(const T& data);

template<typename T>
size_t get_net_size(const std::vector<T>& data);

template<>
size_t get_net_size<std::string>(const std::string& data);

#pragma pack(push, 1)

// Message that represents a packet.
template <typename T, size_t MagicHeaderSize = 4>
class Message {
	friend class MessageDispatcher<T, MagicHeaderSize>;
	friend class MessageReader<T, MagicHeaderSize>;
public:
	inline static constexpr size_t kMagicHeaderSize{ MagicHeaderSize };
	inline static constexpr size_t kMaxMessageBodySize{ 4096 };

	struct MessageHeader {
		char magic_header[kMagicHeaderSize]{};
		T id{};
		uint32_t body_size{ 0 };
	};

	inline static constexpr size_t kHeaderSize{ sizeof(MessageHeader) };
	static_assert(sizeof(MessageHeader) == kMagicHeaderSize + sizeof(uint32_t) + sizeof(T), "Message header should be packed.");

	friend std::ostream& operator << (std::ostream& os, const Message& msg)
	{
		os << "Message: { magic_header = " << std::string(header_.magic_header, kMagicHeaderSize)
			<< ", id = " << header_.id << ", size = " << header_.size << "}";
	}

	template<typename DataType>
	friend Message& operator << (Message& msg, const DataType& data)
	{
		static_assert(std::is_standard_layout<DataType>::value, "Can not serialize non-standard_layout type");

		msg.header_.body_size += sizeof(DataType);
		assert(msg.header_.body_size <= kMaxMessageBodySize);

		size_t write_ptr = msg.body_.size();
		msg.body_.resize(msg.body_.size() + sizeof(DataType));
		memcpy(&msg.body_[write_ptr], &data, sizeof(DataType));

		return msg;
	}

	friend Message& operator << (Message& msg, const std::string& data)
	{
		size_t string_content_size = sizeof(std::string::value_type) * data.size();
		size_t final_body_size = msg.body_.size() + get_net_size(data);
		assert(final_body_size <= Message::kMaxMessageBodySize);
		msg << static_cast<uint16_t>(data.size());

		size_t write_ptr = msg.body_.size();
		msg.body_.resize(final_body_size);
		memcpy(&msg.body_[write_ptr], data.c_str(), string_content_size);
		msg.header_.body_size = static_cast<uint32_t>(final_body_size);

		return msg;
	}

	template<typename DataType>
	friend Message& operator << (Message& msg, const std::vector<DataType>& data)
	{
		size_t final_body_size = 0;

		if (!msg.nested_mode_) {
			final_body_size = msg.body_.size() + get_net_size(data);
			assert(final_body_size <= Message::kMaxMessageBodySize);
			msg.body_.reserve(final_body_size);
			msg.nested_mode_ = true;
		}

		msg << static_cast<uint16_t>(data.size());
		for (const auto& d : data) {
			msg << d;
		}

		if (final_body_size > 0) {
			assert(msg.header_.body_size == final_body_size && final_body_size == msg.body_.size());
			msg.nested_mode_ = false;
		}

		return msg;
	}

	void Send(std::vector<uint8_t>& send_buffer)
	{
		size_t write_ptr = send_buffer.size();
		send_buffer.resize(write_ptr + sizeof(MessageHeader) + body_.size());

		memcpy(send_buffer.data() + write_ptr, &header_, sizeof(MessageHeader));
		write_ptr += sizeof(MessageHeader);

		memcpy(send_buffer.data() + write_ptr, body_.data(), body_.size());
	}

	Message(const std::string& magic_header, T id)
	{
		assert(magic_header.size() == kMagicHeaderSize);
		memcpy(header_.magic_header, magic_header.c_str(), kMagicHeaderSize);
		header_.id = id;
	}

private:
	MessageHeader header_;
	std::vector<uint8_t> body_;
	bool nested_mode_{ false };
};
#pragma pack(pop)

// ostream like data structure that helps deserialize packet contents into variables.
template <typename T, size_t MagicHeaderSize>
class MessageReader {
public:
	using MessageType = Message<T, MagicHeaderSize>;

	template<typename DataType>
	friend MessageReader& operator >> (MessageReader& reader, DataType& data)
	{
		static_assert(std::is_standard_layout<DataType>::value, "Can not deserialize non-standard_layout type");
		assert(reader.read_ptr_ + sizeof(DataType) <= reader.buffer_.size());

		memcpy(&data, &reader.buffer_[reader.read_ptr_], sizeof(DataType));
		reader.read_ptr_ += sizeof(DataType);

		return reader;
	}

	friend MessageReader& operator >> (MessageReader& reader, std::string& data)
	{
		uint16_t str_size;
		reader >> str_size;

		data.resize(str_size);
		memcpy(data.data(), &reader.buffer_[reader.read_ptr_], str_size * sizeof(std::string::value_type));
		reader.read_ptr_ += str_size * sizeof(std::string::value_type);

		return reader;
	}

	template<typename DataType>
	friend MessageReader& operator >> (MessageReader& reader, std::vector<DataType>& data)
	{
		uint16_t vector_size;
		reader >> vector_size;

		data.resize(vector_size);
		for (DataType& d : data) {
			reader >> d;
		}

		return reader;
	}

	MessageReader(const MessageType& message) :
		buffer_(message.body_)
	{}

	MessageReader(const std::vector<uint8_t>& buffer) :
		buffer_(buffer)
	{}

private:
	std::vector<uint8_t> buffer_;
	size_t read_ptr_{ 0 };
};

// Parse input buffer and dispatch packet received to callbacks.
template <typename T, size_t MagicHeaderSize>
class MessageDispatcher {
public:
	using MessageType = Message<T, MagicHeaderSize>;
	using MessageReaderType = MessageReader<T, MagicHeaderSize>;

	MessageDispatcher(const std::string& magic_header):
	magic_header_(magic_header)
	{
		assert(magic_header.size() == MagicHeaderSize);
	}

	void RegisterMessageCallback(T id, const std::function<void(MessageReader<T, MagicHeaderSize>)>& callback)
	{
		auto it = rpc_dispatch_table_.find(id);
		if (it != rpc_dispatch_table_.end()) {
			throw std::runtime_error("");
		}

		rpc_dispatch_table_[id] = callback;
	}

	bool OnRecv(std::vector<uint8_t>& recv_buffer)
	{
		size_t read_ptr = 0;
		constexpr size_t kHeaderSize = MessageType::kHeaderSize;
		using Header = typename MessageType::MessageHeader;

		while (true) {
			// Not enough data to complete packet header
			if (read_ptr + kHeaderSize > recv_buffer.size()) {
				break;
			}

			char magic_header[MagicHeaderSize];
			T id;
			int body_size;

			memcpy(magic_header, &recv_buffer[read_ptr + offsetof(Header, magic_header)], MagicHeaderSize);
			memcpy(&id, &recv_buffer[read_ptr + offsetof(Header, id)], sizeof(T));
			memcpy(&body_size, &recv_buffer[read_ptr + offsetof(Header, body_size)], sizeof(int));

			std::string magic_header_str(magic_header, MagicHeaderSize);
			if (magic_header_ != magic_header_str) {
				std::cerr << "Magic header mismatch, header: " << magic_header_str << std::endl;
				return false;
			}

			auto it = rpc_dispatch_table_.find(id);
			if (it == rpc_dispatch_table_.end()) {
				std::cerr << "No rpc corresponds to the packet, id: " << static_cast<uint8_t>(id) << std::endl;
				return false;
			}

			if (body_size > MessageType::kMaxMessageBodySize) {
				std::cerr << "Packet size too large, size: " << body_size << std::endl;
				return false;
			}

			// Not enough data to complete packet body
			if (read_ptr + kHeaderSize + body_size > recv_buffer.size()) {
				break;
			}

			// Call back
			(*it).second(MessageReaderType(std::vector<uint8_t>(recv_buffer.begin() + read_ptr + kHeaderSize,
				recv_buffer.begin() + read_ptr + kHeaderSize + body_size)));

			// Move read pointer
			read_ptr += kHeaderSize + body_size;
		}

		// Consume recv buffer
		recv_buffer.erase(recv_buffer.begin(), recv_buffer.begin() + read_ptr);
		return true;
	}

private:
	std::string magic_header_;
	std::unordered_map<T, std::function<void(MessageReaderType)>> rpc_dispatch_table_;
};
