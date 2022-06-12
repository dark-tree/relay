
#include "users.hpp"

std::unordered_map<uint32_t, Group> groups;
std::shared_mutex groups_mutex;

uint32_t User::next = 1;
uint32_t Group::next = 1;

void user_safe_exit(std::shared_ptr<User> user) {
	std::unique_lock<std::shared_mutex> lock(groups_mutex);

	if (user->level == 2) {
		groups.at(user->gid).close();
	}

	if (user->level == 1) {
		groups.at(user->gid).remove(user);
	}
}

void Group::join(std::shared_ptr<User> user) {
	// can't join a group without host, silenty ignore
	if (!members.empty()) {
		members.push_back(user);

		user->level = 1;
		user->gid = this->gid;

		// notify group host
		PacketWriter(R2U_JOIN).write(user->uid).pack().send(host->sock);
	}
}

void Group::remove(std::shared_ptr<User> user) {
	members.erase(std::ranges::find(members.begin(), members.end(), user));

	user->level = 0;
	user->gid = 0;

	// notify group host
	PacketWriter(R2U_LEFT).write(user->uid).pack().send(host->sock);
}

void Group::close() {
	for(auto& user : members) {
		user->level = 0;
		user->gid = 0;
	}

	members.clear();

	host->level = 0;
	host->gid = 0;
	host.reset();

	groups.erase(gid);
}

void Group::brodcast(const Packet& packet) {
	for(auto& user : members) {
		packet.send(user->sock);
	}
}

void Group::send(uint32_t uid, const Packet& packet) {
	packet.send(get_user(uid)->sock);
}

