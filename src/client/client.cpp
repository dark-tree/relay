
#include "core.hpp"
#include "packets.hpp"
#include "parser.hpp"
#include "writer.hpp"

int main() {

	logger::info("Connecting...");

	asio::io_context context;
	tcp::socket socket(context);
	tcp::resolver resolver(context);
	auto endpoints = resolver.resolve("localhost", "9698");

	try{
		asio::connect(socket, endpoints);
	} catch(...) {
		logger::fatal("Failed to bind to port!");
		exit(1);
	}

	std::thread socket_thread([&] () -> void {
		try{
			while (true) {
				// load packet header
				ClientPacketHead head;
				asio::read(socket, asio::buffer((uint8_t*) &head, sizeof(ClientPacketHead)));

				// load packet body
				uint8_t body[head.size];
				asio::read(socket, asio::buffer(body, head.size));

				// execute command
				head.accept(body);
			}
		} catch(...) {
			logger::fatal("Connection closed by relay!");
			exit(2);
		}
	});

	run_input_processor([&] (auto& parts) {

		match_command(parts, "help", 1, [] (auto& parts) {
			std::cout << "List of avaible commands:" << std::endl;
			std::cout << "bin              - toggle binary print mode" << std::endl;
			std::cout << "help             - print this help page" << std::endl;
			std::cout << "quit             - stop this application" << std::endl;
			std::cout << "make             - create a user group" << std::endl;
			std::cout << "join [gid]       - join a user group" << std::endl;
			std::cout << "exit             - exit a user group" << std::endl;
			std::cout << "broadcast [msg]  - brodcast a message to all group users" << std::endl;
			std::cout << "send [uid] [msg] - send a message to a specific group user" << std::endl;
		});

		match_command(parts, "bin", 1, [] (auto& parts) {
			binary = !binary;
		});

		match_command(parts, "make", 1, [&socket] (auto& parts) {
			PacketWriter(U2R_MAKE).pack().send(socket);
		});

		match_command(parts, "join", 2, [&socket] (auto& parts) {
			uint32_t gid = std::stoi(parts[1]);
			PacketWriter(U2R_JOIN).write(gid).pack().send(socket);
		});

		match_command(parts, "exit", 1, [&socket] (auto& parts) {
			PacketWriter(U2R_QUIT).pack().send(socket);
		});

		match_command(parts, "broadcast", 2, [&socket] (auto& parts) {
			PacketWriter(U2R_BROD).write(NULL_USER).write(parts[1]).pack().send(socket);
		});

		match_command(parts, "send", 3, [&socket] (auto& parts) {
			uint32_t uid = std::stoi(parts[1]);
			PacketWriter(U2R_SEND).write(uid).write(parts[2]).pack().send(socket);
		});

		throw std::invalid_argument("No such command");
	});

	return 0;
}

