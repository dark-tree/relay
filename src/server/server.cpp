
#include "packets.hpp"
#include "parser.hpp"

std::mutex users_mutex;
std::vector<std::shared_ptr<User>> users;

const char* levels[] = {
	"no_one",
	"member",
	"host"
};

void session(std::shared_ptr<User> user) {
	logger::info("User #", user->uid, " connected");

	users_mutex.lock();
	users.push_back(user);
	users_mutex.unlock();

	PacketWriter(R2U_WELC).write(user->uid).write(URP_VERSION).pack().send(user->sock);

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
	users.erase(std::ranges::find(users.begin(), users.end(), user));
	users_mutex.unlock();

	user_safe_exit(user);
	logger::info("User #", user->uid, " disconnected");
}

int main(int argc, char* argv[]) {

	assert (sizeof(ServerPacketHead) == 3);

	// asynchonusly watch for user input
	std::thread input_thread([&] () -> void {
		run_input_processor([] (auto& parts) {

			match_command(parts, "help", 1, [] (auto& parts) {
				std::cout << "List of avaible commands:" << std::endl;
				std::cout << "help             - print this help page" << std::endl;
				std::cout << "quit             - stop this application" << std::endl;
				std::cout << "users            - list all users" << std::endl;
				std::cout << "groups           - list all groups" << std::endl;
				std::cout << "members [gid]    - list all group members" << std::endl;
			});

			match_command(parts, "users", 1, [&] (auto& parts) {
				std::cout << "There are: " << users.size() << " users connected\n";
				for (auto& user : users) {
					std::cout << " - user #" << user->uid << " (" << levels[user->level] << ")" << std::endl;
				}
			});

			match_command(parts, "groups", 1, [&] (auto& parts) {
				std::cout << "There are: " << groups.size() << " currently open user groups\n";
				for (auto& entry : groups) {
					std::cout << " - group #" << entry.first << " (" << entry.second.members.size() << " members) host_uid=" << entry.second.host_uid() << std::endl;
				}
			});

			match_command(parts, "members", 2, [&] (auto& parts) {
				uint32_t gid = std::stoi(parts[1]);

				if (groups.contains(gid)) {
					Group& group = groups.at(gid);

					std::cout << "There are: " << group.members.size() << " members in user group #" << gid << "\n";
					for (auto& member : group.members) {
						std::cout << " - user #" << member->uid << (member->level == 2 ? " (host)" : "") << std::endl;
					}
				} else {
					std::cout << "No such group found\n";
				}
			});


			throw std::invalid_argument("No such command");
		});
	});

	// start the server
	std::vector<tcp::socket> sockets;
	asio::io_context context;
	tcp::acceptor acceptor(context, tcp::endpoint(tcp::v4(), 9698));

	logger::info("Listening on port 9698...");

	// accept new sessions
	while (true) {
		std::thread(session, std::make_shared<User>(User(std::move(acceptor.accept())))).detach();
	}

	return 0;
}

