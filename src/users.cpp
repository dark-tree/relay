
#include "users.hpp"

std::unordered_map<uint32_t, Group> groups;

uint32_t User::next = 1;
uint32_t Group::next = 1;

void user_safe_exit(std::shared_ptr<User> user) {
	if (user->level == 2) {
		groups.at(user->gid).close();
	}

	if (user->level == 1) {
		groups.at(user->gid).remove(user);
	}
}

