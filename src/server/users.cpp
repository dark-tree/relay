
#include "users.hpp"

std::unordered_map<uint32_t, Group, ByteHash> groups;
std::shared_mutex groups_mutex;

Random User::generator {};
Random Group::generator {};

void user_safe_exit(std::shared_ptr<User> user) {
	std::unique_lock<std::shared_mutex> lock(groups_mutex);

	if (user->level == LEVEL_HOST) {
		groups.at(user->gid).close();
	}

	if (user->level == LEVEL_MEMBER) {
		groups.at(user->gid).remove(user);
	}
}

uint8_t Group::join(std::shared_ptr<User> user, uint32_t password) {
	if (this->password != password) {
		logger::info("User #", user->uid, " attempted to join group #", gid, ", but provided invalid password");
		return MADE_STATUS_PASS;
	}

	if (this->flags & GROUP_FLAG_LOCK) {
		logger::info("User #", user->uid, " attempted to join group #", gid, ", but the group is locked");
		return MADE_STATUS_LOCK;
	}

	if (members.empty()) {
		logger::warn("User #", user->uid, " attempted to join group #", gid, ", but there was no host");
		return MADE_STATUS_FAIL;
	}

	logger::info("User #", user->uid, " joined group #", gid);
	members.push_back(user);

	user->level = LEVEL_MEMBER;
	user->gid = this->gid;

	// notify group host
	PacketWriter(R2U_JOIN).write32(user->uid).pack().send(host->sock);
	return MADE_STATUS_JOIN;
}

void Group::remove(std::shared_ptr<User> user) {
	logger::info("User #", user->uid, " left group #", gid);
	members.erase(std::ranges::find(members.begin(), members.end(), user));

	user->level = LEVEL_NO_ONE;
	user->gid = NULL_GROUP;

	// notify group host and the user
	PacketWriter(R2U_LEFT).write32(user->uid).pack().send(host->sock);
	PacketWriter(R2U_EXIT).pack().send(user->sock);
}

void Group::close() {
	logger::info("Group #", gid, " closed by host");

	for(auto& user : members) {
		user->level = LEVEL_NO_ONE;
		user->gid = NULL_GROUP;
		PacketWriter(R2U_EXIT).pack().send(user->sock);
	}

	members.clear();

	host->level = LEVEL_NO_ONE;
	host->gid = NULL_GROUP;
	host.reset();

	groups.erase(gid);
}

void Group::brodcast(const Packet& packet, uint32_t uid) {
	for(auto& user : members) {
		if (user->uid != uid) {
			packet.send(user->sock);
		}
	}
}

void Group::send(uint32_t uid, const Packet& packet) {
	packet.send(get_user(uid)->sock);
}

