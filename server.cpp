
#include "Connection.hpp"

#include "hex_dump.hpp"
#include "Player.hpp"
#include "Message.hpp"
#include "NetCommon.hpp"

#include <chrono>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include <unordered_map>

int main(int argc, char **argv) {
#ifdef _WIN32
	//when compiled on windows, unhandled exceptions don't have their message printed, which can make debugging simple issues difficult.
	try {
#endif

	//------------ argument parsing ------------

	if (argc != 2) {
		std::cerr << "Usage:\n\t./server <port>" << std::endl;
		return 1;
	}

	//------------ initialization ------------

	Server server(argv[1]);


	//------------ main loop ------------
	constexpr float ServerTick = 1.0f / 10.0f; //TODO: set a server tick that makes sense for your game

	//server state:

	//per-client state:
	// struct PlayerInfo {
	// 	PlayerInfo() {
	// 		static uint32_t next_player_id = 1;
	// 		name = "Player" + std::to_string(next_player_id);
	// 		next_player_id += 1;
	// 	}
	// 	std::string name;

	// 	uint32_t left_presses = 0;
	// 	uint32_t right_presses = 0;
	// 	uint32_t up_presses = 0;
	// 	uint32_t down_presses = 0;

	// 	int32_t total = 0;

	// };
	
	Connection* left_connection { nullptr };
	Player left_player(ChannelId::LEFT_SIDE);
	MessageDispatcher<ChannelId, 4> left_channel_dispatcher { kMagicHeader };

	Connection* right_connection { nullptr };
	Player right_player(ChannelId::RIGHT_SIDE);
	MessageDispatcher<ChannelId, 4> right_channel_dispatcher { kMagicHeader };
	bool game_start { false };

	left_channel_dispatcher.RegisterMessageCallback(ChannelId::LEFT_SIDE, [&](MessageReader<ChannelId, 4> reader){
		left_player.GetMessageDispatcher().OnRecv(reader.GetBuffer());
	});

	right_channel_dispatcher.RegisterMessageCallback(ChannelId::RIGHT_SIDE, [&](MessageReader<ChannelId, 4> reader){
		right_player.GetMessageDispatcher().OnRecv(reader.GetBuffer());
	});
	
	
	while (true) {
		static auto next_tick = std::chrono::steady_clock::now() + std::chrono::duration< double >(ServerTick);
		//process incoming data from clients until a tick has elapsed:
		while (true) {
			auto now = std::chrono::steady_clock::now();
			double remain = std::chrono::duration< double >(next_tick - now).count();
			if (remain < 0.0) {
				next_tick += std::chrono::duration< double >(ServerTick);
				break;
			}
			server.poll([&](Connection *c, Connection::Event evt){
				if (evt == Connection::OnOpen) {
					//client connected:
					if (!left_connection) {
						left_connection = c;
					} else if (!right_connection) {
						right_connection = c;

						// Game starts!
						Message<ChannelId, 4> left_msg(kMagicHeader, ChannelId::CONTROL_CHANNEL);
						left_msg << true;
						left_msg.Send(left_connection->send_buffer);

						Message<ChannelId, 4> right_msg(kMagicHeader, ChannelId::CONTROL_CHANNEL);
						right_msg << true;
						right_msg.Send(left_connection->send_buffer);

						game_start = true;
					} else {
						// TODO
						throw std::runtime_error("Too many clients!");
					}
				} else if (evt == Connection::OnClose) {
					//client disconnected:

					//remove them from the players list:
					// TODO
					throw std::runtime_error("Client disconnected!");
				} else { assert(evt == Connection::OnRecv);
					//got data from client:
					std::cout << "got bytes:\n" << hex_dump(c->recv_buffer); std::cout.flush();

					if (c == left_connection) {
						left_channel_dispatcher.OnRecv(c->recv_buffer);
					} else if (c == right_connection) {
						right_channel_dispatcher.OnRecv(c->recv_buffer);
					} else {
						// TODO
						throw std::runtime_error("Unknown client!");
					}
				}
			}, remain);
		}

		if (!game_start) {
			continue;
		}
	}


	return 0;

#ifdef _WIN32
	} catch (std::exception const &e) {
		std::cerr << "Unhandled exception:\n" << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "Unhandled exception (unknown type)." << std::endl;
		throw;
	}
#endif
}
