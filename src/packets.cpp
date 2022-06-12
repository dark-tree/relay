
#include "packets.hpp"

void PacketHead::accept(uint8_t* body, std::shared_ptr<User> user) {
	switch (type) {

		// user requested new group to be created
		scase(U2R_MAKE, {
			if (user->level == 0) {
				Group group(user);
				std::cout << "INFO: User #" << user->uid << " created group #" << group.gid << "\n";
				PacketWriter(R2U_MADE).write(group.gid).pack().send(user->sock);
				groups.emplace(group.gid, std::move(group));
			}
		});

		// user wants to join a given group
		scase(U2R_JOIN, {
			if (user->level == 0) {
				uint32_t gid = read32(body);
				Group& group = groups.at(gid);
				group.join(user);

				std::cout << "INFO: User #" << user->uid << " joined group #" << group.gid << "\n";
			}
		});

		// user wants to reset it's state
		scase(U2R_QUIT, {
			user_safe_exit(user);
		});

		// users wants to brodcast a message within a group
		scase(U2R_BROD, {
			if (user->level != 0) {
				groups.at(user->gid).brodcast(PacketWriter(R2U_TEXT).write(body, size).pack());
			}
		});

		// users wants to send a message to a specific user within a group
		scase(U2R_SEND, {
			if (user->level != 0) {
				uint32_t uid = read32(body);
				groups.at(user->gid).send(uid, PacketWriter(R2U_TEXT).write(body + 4, size - 4).pack());
			}
		});

	}
}

