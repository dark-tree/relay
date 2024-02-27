
#include "packets.hpp"

using namespace std::chrono_literals;

void ServerPacketHead::accept(uint8_t* body, std::shared_ptr<User> user) {
	switch (type) {

		// user requested new group to be created
		scase(U2R_MAKE, {
			std::this_thread::sleep_for(300ms);

			if (user->level == LEVEL_NO_ONE) {
				std::unique_lock<std::shared_mutex> lock {groups_mutex};

				Group group {user};
				logger::info("User #", user->uid, " created group #", group.gid);
				PacketWriter(R2U_MADE).write32(group.gid).write8(MADE_STATUS_MADE).pack().send(user->sock);
				groups.emplace(group.gid, std::move(group));
			}
		});

		// user wants to join a given group
		scase(U2R_JOIN, {
			std::this_thread::sleep_for(300ms);

			if (user->level == LEVEL_NO_ONE) {
				uint32_t gid = read32(body);
				uint32_t pass = read32(body + 4);

				std::unique_lock<std::shared_mutex> lock {groups_mutex};
				Group& group = groups.at(gid);
				
				uint8_t status = group.join(user, pass);
				PacketWriter(R2U_MADE).write32(group.gid).write8(status).pack().send(user->sock);
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
				uint32_t uid = read32(body);

				std::shared_lock<std::shared_mutex> lock {groups_mutex};
				Group& group = groups.at(user->gid);

				if (group.can_speak(user->uid)) {
					group.brodcast(PacketWriter(R2U_TEXT).write(body + 4, size - 4).pack(), uid);
				}
			}
		});

		// users wants to send a message to a specific user within a group
		scase(U2R_SEND, {
			if (user->level != LEVEL_NO_ONE) {
				uint32_t uid = read32(body);

				std::shared_lock<std::shared_mutex> lock {groups_mutex};
				Group& group = groups.at(user->gid);

				if (group.can_speak(user->uid)) {
					group.send(uid, PacketWriter(R2U_TEXT).write(body + 4, size - 4).pack());
				}
			}
		});

		// users wants to modify some relay-held data
		scase(U2R_SETS, {
			uint32_t key = read32(body);
			uint32_t val = read32(body + 4);

			if (key == DATA_KEY_PASS && user->level == LEVEL_HOST) {
				std::shared_lock<std::shared_mutex> lock {groups_mutex};
				groups.at(user->gid).password = val;
				PacketWriter(R2U_VALS).write32(key).write32(val).pack().send(user->sock);
				break;
			}

			if (key == DATA_KEY_FLAG && user->level == LEVEL_HOST) {
				std::shared_lock<std::shared_mutex> lock {groups_mutex};
				groups.at(user->gid).flags = val;
				PacketWriter(R2U_VALS).write32(key).write32(val).pack().send(user->sock);
				break;
			}
		});

		// users wants to read some relay-held data
		scase(U2R_GETS, {
			uint32_t key = read32(body);
			uint32_t val = 0;

			if (key == DATA_KEY_PASS && user->level != LEVEL_NO_ONE) {
				{
					std::shared_lock<std::shared_mutex> lock {groups_mutex};
					val = groups.at(user->gid).password;
				}

				PacketWriter(R2U_VALS).write32(key).write32(val).pack().send(user->sock);
				break;
			}

			if (key == DATA_KEY_FLAG && user->level != LEVEL_NO_ONE) {
				{
					std::shared_lock<std::shared_mutex> lock {groups_mutex};
					val = groups.at(user->gid).flags;
				}

				PacketWriter(R2U_VALS).write32(key).write32(val).pack().send(user->sock);
				break;
			}
		});

	}
}

