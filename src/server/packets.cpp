
#include "packets.hpp"

void ServerPacketHead::accept(uint8_t* body, std::shared_ptr<User> user) {
	switch (type) {

		// user requested new group to be created
		scase(U2R_MAKE, {
			if (user->level == LEVEL_NO_ONE) {
				std::unique_lock<std::shared_mutex> lock(groups_mutex);

				Group group(user);
				logger::info("User #", user->uid, " created group #", group.gid);
				PacketWriter(R2U_MADE).write(group.gid).pack().send(user->sock);
				groups.emplace(group.gid, std::move(group));
			}
		});

		// user wants to join a given group
		scase(U2R_JOIN, {
			if (user->level == LEVEL_NO_ONE) {
				uint32_t gid = read32(body);

				std::unique_lock<std::shared_mutex> lock(groups_mutex);
				Group& group = groups.at(gid);
				group.join(user);
			}
		});

		// user wants to reset its state
		scase(U2R_QUIT, {
			if (user->level != LEVEL_NO_ONE) {
				user_safe_exit(user);
			}
		});

		// users wants to brodcast a message within a group
		scase(U2R_BROD, {
			if (user->level != LEVEL_NO_ONE) {
				std::shared_lock<std::shared_mutex> lock(groups_mutex);
				groups.at(user->gid).brodcast(PacketWriter(R2U_TEXT).write(body, size).pack(), NULL_USER);
			}
		});

		// users wants to send a message to a specific user within a group
		scase(U2R_SEND, {
			if (user->level != LEVEL_NO_ONE) {
				uint32_t uid = read32(body);
				std::shared_lock<std::shared_mutex> lock(groups_mutex);
				groups.at(user->gid).send(uid, PacketWriter(R2U_TEXT).write(body + 4, size - 4).pack());
			}
		});

	}
}

