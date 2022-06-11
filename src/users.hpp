
#include "core.hpp"

class User {

	private:
		static uint32_t next;

	public:
		const std::shared_ptr<tcp::socket> sock;
		const uint32_t uid;

		uint8_t level;
		uint32_t gid;

		User(std::shared_ptr<tcp::socket> sock) : sock(sock), uid(next ++) {
			this->level = 0;
			this->gid = 0;
		}

};

void user_safe_exit(std::shared_ptr<User> user);

class Group {

	private:
		static uint32_t next;

	public:
		const uint32_t gid;

		std::shared_ptr<User> host;
		std::vector<std::shared_ptr<User>> members;

		Group(std::shared_ptr<User> user) : host(host), gid(next ++) {
			members.push_back(user);

			user->level = 2;
			user->gid = this->gid;
		}

		void join(std::shared_ptr<User> user) {
			members.push_back(user);

			user->level = 1;
			user->gid = this->gid;
		}

		void remove(std::shared_ptr<User> user) {
			members.erase(std::find(members.begin(), members.end(), user));

			user->level = 0;
			user->gid = 0;
		}

		void close() {
			for(auto& user : members) {
				user->level = 0;
				user->gid = 0;
			}

			members.clear();

			host->level = 0;
			host->gid = 0;
			host.reset();
		}

		void brodcast(uint8_t* data, uint16_t size) {
			for(auto& user : members) {
				asio::write(*(user->sock), asio::buffer(data, size));
			}
		}

};

extern std::unordered_map<uint32_t, Group> groups;

