
#pragma once

#include "users.hpp"
#include "core.hpp"

struct __attribute__((__packed__)) packet_head_t {

	private:
		uint32_t read32(uint8_t* data) {
			uint32_t val;
			memcpy((uint8_t*) &val, data, 4);
			return val;
		}

	public:
		const uint8_t type;
		const uint16_t size;

		packet_head_t() : type(0), size(0) {
			// dummy implementation, deserialize with memcpy
		}

		void accept(uint8_t* body, std::shared_ptr<User> user) {
			switch (type) {
				scase(U2R_MAKE, {
					Group group(user);
					std::cout << "INFO: User #" << user->uid << " created group #" << group.gid << "\n";
					PacketWriter(R2U_MADE).write(group.gid).pack().send(user->sock);
					groups.emplace(group.gid, std::move(group));
				});

				scase(U2R_JOIN, {
					uint32_t gid = read32(body);
					Group& group = groups.at(gid);
					group.join(user);

					std::cout << "INFO: User #" << user->uid << " joined group #" << group.gid << "\n";
				});

				scase(U2R_QUIT, {
					user_safe_exit(user);
				});

				scase(U2R_BROD, {
					groups.at(user->gid).brodcast(PacketWriter(R2U_TEXT).write(body, size).pack());
				});

				scase(U2R_SEND, {
					uint32_t uid = read32(body);
					groups.at(user->gid).send(uid, PacketWriter(R2U_TEXT).write(body + 4, size - 4).pack());
				});

			}
		}

};

