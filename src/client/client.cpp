
#include "core.hpp"
#include "packets.hpp"
#include "parser.hpp"
#include "writer.hpp"

uint32_t get_key_code(const char* key) {
	if (strcmp("group.password", key) == 0) return DATA_KEY_PASS;
	if (strcmp("group.flags", key) == 0) return DATA_KEY_FLAG;
	
	throw std::invalid_argument {"No such key code"};
}

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
			std::cout << "List of available commands:" << std::endl;
			std::cout << "  bin                Toggle binary print mode" << std::endl;
			std::cout << "  help               Print this help page" << std::endl;
			std::cout << "  quit               Stop this application" << std::endl;
			std::cout << "  make               Create a user group" << std::endl;
			std::cout << "  join [gid] [pass]  Join a user group" << std::endl;
			std::cout << "  exit               Exit a user group" << std::endl;
			std::cout << "  broadcast [msg]    Brodcast a message to all group users" << std::endl;
			std::cout << "  send [uid] [msg]   Send a message to a specific group user" << std::endl;
			std::cout << "  set [key] [val]    Set setting 'key' to the given value" << std::endl;
			std::cout << "  get [key]          Get the value of setting 'key'" << std::endl;
		});

		match_command(parts, "bin", 1, [] (auto& parts) {
			binary = !binary;
		});

		match_command(parts, "make", 1, [&socket] (auto& parts) {
			PacketWriter(U2R_MAKE).pack().send(socket);
		});

		match_command(parts, "join", 3, [&socket] (auto& parts) {
			uint32_t gid = std::stol(parts[1]);
			uint16_t pass = std::stol(parts[2]);
			PacketWriter(U2R_JOIN).write32(gid).write32(pass).pack().send(socket);
		});

		match_command(parts, "exit", 1, [&socket] (auto& parts) {
			PacketWriter(U2R_QUIT).pack().send(socket);
		});

		match_command(parts, "broadcast", 2, [&socket] (auto& parts) {
			PacketWriter(U2R_BROD).write32(NULL_USER).write(parts[1]).pack().send(socket);
		});

		match_command(parts, "send", 3, [&socket] (auto& parts) {
			uint32_t uid = std::stol(parts[1]);
			PacketWriter(U2R_SEND).write32(uid).write(parts[2]).pack().send(socket);
		});

		match_command(parts, "set", 3, [&socket] (auto& parts) {
			uint32_t key = get_key_code(parts[1].c_str());
			uint32_t val = std::stol(parts[2]);
			PacketWriter(U2R_SETS).write32(key).write32(val).pack().send(socket);
		});

		match_command(parts, "get", 2, [&socket] (auto& parts) {
			uint32_t key = get_key_code(parts[1].c_str());
			PacketWriter(U2R_GETS).write32(key).pack().send(socket);
		});

		throw std::invalid_argument {"No such command"};
	});

	return 0;
}

