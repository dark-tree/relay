
#include "packets.hpp"

void session(std::shared_ptr<User> user) {
	std::cout << "INFO: User #" << user->uid << " connected\n";

	PacketWriter(R2U_WELC).write(user->uid).write(URP_VERSION).pack().send(user->sock);

	try{
		while (true) {
			// load packet header
			PacketHead head;
			asio::read(user->sock, asio::buffer((uint8_t*) &head, sizeof(PacketHead)));

			// load packet body
			uint8_t body[head.size];
			asio::read(user->sock, asio::buffer(body, head.size));

			// execute command
			head.accept(body, user);
		}
	} catch(...) {}

	user_safe_exit(user);
	std::cout << "INFO: User #" << user->uid << " disconnected\n";
}

int main(int argc, char* argv[]) {

	assert (sizeof(PacketHead) == 3);

	// asynchonusly watch for user input
	std::thread input_thread([&] () -> void {
		while (true) {
			std::string input;
			std::cin >> input;

			if (input == "exit" | input == "quit") {
				exit(0);
			}
		}
	});

	// start the server
	std::vector<tcp::socket> sockets;
	asio::io_context io_context;
	tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 9698));

	std::cout << "INFO: Listening on port 9698...\n";

	// accept new sessions
	while (true) {
		std::thread(session, std::make_shared<User>(User(std::move(acceptor.accept())))).detach();
	}

	return 0;
}

