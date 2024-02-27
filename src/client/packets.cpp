
#include "packets.hpp"

bool binary = false;

std::string buffer_to_string(char* data, int size) {
	if (binary) {
		std::stringstream ss;

		for (int i = 0; i < size; i ++) {
			ss << std::hex << (int) data[i] << " ";
		}

		return ss.str();
	}

	return std::string(data, size);
}

const char* get_key_string(uint32_t key) {
	if (key == DATA_KEY_PASS) return "group.password";
	if (key == DATA_KEY_FLAG) return "group.flags";

	return "undefined";
}

void ClientPacketHead::accept(uint8_t* body) {
	switch (type) {

		// recived massage from other user in the group
		scase(R2U_WELC, {
			logger::info("Connection established (uid: ", read32(body), "), using URP v", read32(body + 4));
		});

		// recived massage from other user in the group
		scase(R2U_TEXT, {
			logger::info("Received message: ", buffer_to_string((char*) body, size));
		});

		// group creation confirmation
		scase(R2U_MADE, {
			uint32_t gid = read32(body);
			uint8_t status = read8(body + 4);

			switch (status) {
				scase(MADE_STATUS_MADE, {
					logger::info("New group created (gid: ", gid, ")");
				});

				scase(MADE_STATUS_JOIN, {
					logger::info("New group joined (gid: ", gid, ")");
				});

				scase(MADE_STATUS_PASS, {
					logger::info("Failed to join, invalid password");
				});

				scase(MADE_STATUS_LOCK, {
					logger::info("Failed to join, group closed");
				});

				scase(MADE_STATUS_FAIL, {
					logger::info("Failed to join, relay error");
				});
			}
		});

		// user join notification
		scase(R2U_JOIN, {
			logger::info("User joined current group (uid: ", read32(body), ")");
		});

		// user left notification
		scase(R2U_LEFT, {
			logger::info("User left current group (uid: ", read32(body), ")");
		});

		// data update
		scase(R2U_VALS, {
			logger::info("Value of: ", get_key_string(read32(body)), " is now: ", read32(body + 4));
		});

	}
}

