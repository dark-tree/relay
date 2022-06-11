
#include "users.hpp"
#include "core.hpp"

#define U2R_MAKE 0x00
#define U2R_JOIN 0x01
#define U2R_QUIT 0x02
#define U2R_BROD 0x03

#define R2U_WELC 0x10

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
					groups.at(user->gid).brodcast(body, size);
				});

			}
		}

};

