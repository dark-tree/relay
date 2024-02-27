
#pragma once

#include "writer.hpp"
#include "core.hpp"
#include "random.hpp"

class User {

	public:
		static Random generator;

		tcp::socket sock;
		const uint32_t uid;

		uint8_t level;
		uint32_t gid;

		User(tcp::socket sock) 
		: sock(std::move(sock)), uid(generator.next()), level(LEVEL_NO_ONE), gid(NULL_GROUP) {}

};

struct ByteHash {
	size_t operator () (uint32_t value) const {
		return (value & 0xFF000000) >> 24;
	}
};

void user_safe_exit(std::shared_ptr<User> user);

class Group {

	private:
		std::shared_ptr<User> get_user(uint32_t uid) {
			return *std::ranges::find_if(members.begin(), members.end(), [uid] (auto& val) -> bool {
				return val->uid == uid;
			});
		}

		std::shared_ptr<User> host;

	public:
		static Random generator;

		const uint32_t gid;
		uint32_t password;
		uint32_t flags;
		std::vector<std::shared_ptr<User>> members;

		Group(std::shared_ptr<User> user) 
		: host(user), gid(generator.next()), password(0), flags(0) {
			members.push_back(user);

			user->level = LEVEL_HOST;
			user->gid = this->gid;
		}

		/// get host uid
		uint32_t host_uid() const {
			return host->uid;
		}

		bool can_speak(uint32_t uid) const {
			return (uid == host->uid) || !(flags & GROUP_FLAG_HOST);
		}

		/// add a user to this group
		uint8_t join(std::shared_ptr<User> user, uint32_t password);

		/// remove a user from this group
		void remove(std::shared_ptr<User> user);

		/// close this group (called when host leaves)
		void close();

		/// brodcast a message to all members, except for the given uid
		void brodcast(const Packet& packet, uint32_t uid);

		/// send a message to a member with given uid
		void send(uint32_t uid, const Packet& packet);

};

extern std::unordered_map<uint32_t, Group, ByteHash> groups;
extern std::shared_mutex groups_mutex;

