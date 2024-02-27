
#include "packets.hpp"
#include "parser.hpp"
#include "users.hpp"

std::mutex users_mutex;
std::unordered_map<uint32_t, std::shared_ptr<User>, ByteHash> users;

const char* levels[] = {
	"no_one",
	"member",
	"host"
};

void session(std::shared_ptr<User> user) {
	logger::info("User #", user->uid, " connected");

	users_mutex.lock();
	users[user->uid] = user;
	users_mutex.unlock();

	PacketWriter(R2U_WELC).write32(user->uid).write32(URP_VERSION).pack().send(user->sock);

	try{
		while (true) {
			// load packet header
			ServerPacketHead head;
			asio::read(user->sock, asio::buffer((uint8_t*) &head, sizeof(ServerPacketHead)));

			// load packet body
			uint8_t body[head.size];
			asio::read(user->sock, asio::buffer(body, head.size));

			// execute command
			head.accept(body, user);
		}
	} catch(...) { }

	users_mutex.lock();
	users.erase(user->uid);
	users_mutex.unlock();

	user_safe_exit(user);
	logger::info("User #", user->uid, " disconnected");
}

int main(int argc, char* argv[]) {

	assert (sizeof(ServerPacketHead) == 3);

	// initizlize random generators 
	{
		std::random_device device;
		std::mt19937 generator {device()};

		User::generator.init(generator);
		Group::generator.init(generator);
	}

	// asynchonusly watch for user input
	std::thread input_thread([&] () -> void {
		run_input_processor([] (auto& parts) {

			match_command(parts, "help", 1, [] (auto& parts) {
				std::cout << "List of available commands:" << std::endl;
				std::cout << "  help           Print this help page" << std::endl;
				std::cout << "  quit           Stop this application" << std::endl;
				std::cout << "  users          List all users" << std::endl;
				std::cout << "  groups         List all groups" << std::endl;
				std::cout << "  members [gid]  List all group members" << std::endl;
			});

			match_command(parts, "users", 1, [&] (auto& parts) {
				std::cout << "There are: " << users.size() << " users connected\n";
				for (auto const& [key, user] : users) {
					std::cout << "  user #" << user->uid << " (" << levels[user->level] << ")" << std::endl;
				}
			});

			match_command(parts, "groups", 1, [&] (auto& parts) {
				std::cout << "There are: " << groups.size() << " currently open user groups\n";
				for (auto const& [key, group] : groups) {
					std::cout << "  group #" << key << " (" << group.members.size() << " members) host_uid=" << group.host_uid() << std::endl;
				}
			});

			match_command(parts, "members", 2, [&] (auto& parts) {
				uint32_t gid = std::stol(parts[1]);

				if (groups.contains(gid)) {
					Group& group = groups.at(gid);

					std::cout << "There are: " << group.members.size() << " members in user group #" << gid << "\n";
					for (auto& member : group.members) {
						std::cout << "  user #" << member->uid << (member->level == 2 ? " (host)" : "") << std::endl;
					}
				} else {
					std::cout << "No such group found\n";
				}
			});


			throw std::invalid_argument {"No such command"};
		});
	});

	// start the server
	std::vector<tcp::socket> sockets;
	asio::io_context context;
	tcp::acceptor acceptor {context, tcp::endpoint {tcp::v4(), 9698}};

	logger::info("Listening on port 9698...");

	// accept new sessions
	while (true) {
		std::thread(session, std::make_shared<User>(User (std::move(acceptor.accept())))).detach();
	}

	return 0;
}

