
#pragma once

#include "writer.hpp"
#include "core.hpp"

class User {

	private:
		static uint32_t next;

	public:
		tcp::socket sock;
		const uint32_t uid;

		uint8_t level;
		uint32_t gid;

		User(tcp::socket sock) : sock(std::move(sock)), uid(next ++) {
			this->level = 0;
			this->gid = 0;
		}

};

void user_safe_exit(std::shared_ptr<User> user);

class Group {

	private:
		static uint32_t next;

		std::shared_ptr<User> get_user(uint32_t uid) {
			return *std::ranges::find_if(members.begin(), members.end(), [uid] (auto& val) -> bool {
				return val->uid == uid;
			});
		}

		std::shared_ptr<User> host;

	public:
		const uint32_t gid;
		std::vector<std::shared_ptr<User>> members;

		Group(std::shared_ptr<User> user) : host(user), gid(next ++) {
			members.push_back(user);

			user->level = 2;
			user->gid = this->gid;
		}

		/// get host uid
		uint32_t host_uid() {
			return host->uid;
		}

		/// add a user to this group
		void join(std::shared_ptr<User> user);

		/// remove a user from this group
		void remove(std::shared_ptr<User> user);

		/// close this group (called when host leaves)
		void close();

		/// brodcast a message to all members, except for the given uid
		void brodcast(const Packet& packet, uint32_t uid);

		/// send a message to a member with given uid
		void send(uint32_t uid, const Packet& packet);

};

extern std::unordered_map<uint32_t, Group> groups;
extern std::shared_mutex groups_mutex;

