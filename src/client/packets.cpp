
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
			logger::info("New group created (gid: ", read32(body), ")");
		});

		// user join notification
		scase(R2U_JOIN, {
			logger::info("User joined current group (uid: ", read32(body), ")");
		});

		// user left notification
		scase(R2U_LEFT, {
			logger::info("User left current group (uid: ", read32(body), ")");
		});

	}
}

