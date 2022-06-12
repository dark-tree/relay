
#pragma once

#include "writer.hpp"

#include "core.hpp"

class User {

	private:
		static uint32_t next;

	public:
		/* const */ tcp::socket sock;
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

	public:
		const uint32_t gid;

		std::shared_ptr<User> host;
		std::vector<std::shared_ptr<User>> members;

		Group(std::shared_ptr<User> user) : host(user), gid(next ++) {
			members.push_back(user);

			user->level = 2;
			user->gid = this->gid;
		}

		void join(std::shared_ptr<User> user) {
			members.push_back(user);

			user->level = 1;
			user->gid = this->gid;

			// notify group host
			PacketWriter(R2U_JOIN).write(user->uid).pack().send(host->sock);
		}

		void remove(std::shared_ptr<User> user) {
			members.erase(std::ranges::find(members.begin(), members.end(), user));

			user->level = 0;
			user->gid = 0;

			// notify group host
			PacketWriter(R2U_LEFT).write(user->uid).pack().send(host->sock);
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

		void brodcast(const Packet& packet) {
			for(auto& user : members) {
				packet.send(user->sock);
			}
		}

		void send(uint32_t uid, const Packet& packet) {
			packet.send(get_user(uid)->sock);
		}

};

extern std::unordered_map<uint32_t, Group> groups;

