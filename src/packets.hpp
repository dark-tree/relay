
#pragma once

#include "users.hpp"
#include "core.hpp"

#define U2R_MAKE 0x00 // create new user group
#define U2R_JOIN 0x01 // join a user group
#define U2R_QUIT 0x02 // exit a user group
#define U2R_BROD 0x03 // brodcast a message in a user group
#define U2R_SEND 0x04 // send a message to a specific user of a user group

#define R2U_WELC 0x10 // send to newly joined users
#define R2U_TEXT 0x11 // message packet
#define R2U_MADE 0x12 // message send to a host of a newly made user group
#define R2U_JOIN 0x13 // message send to a host when a member joins
#define R2U_LEFT 0x14 // message send to a host when a member leaves

#define scase(val, ...) case val: { __VA_ARGS__ } break;

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

