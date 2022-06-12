
#include "packets.hpp"

void ClientPacketHead::accept(uint8_t* body) {
	switch (type) {

		// recived massage from other user in the group
		scase(R2U_WELC, {
			std::cout << "INFO: Connection established (uid: " << read32(body) << "), using URP v" << read32(body + 4) << std::endl;
		});

		// recived massage from other user in the group
		scase(R2U_TEXT, {
			std::cout << "INFO: Recived message: " << std::string((char*) body, size) << std::endl;
		});

		// group creation confirmation
		scase(R2U_MADE, {
			std::cout << "INFO: New group created (gid: " << read32(body) << ")" << std::endl;
		});

		// user join notification
		scase(R2U_JOIN, {
			std::cout << "INFO: User joined current group (uid: " << read32(body) << ")" << std::endl;
		});

		// user left notification
		scase(R2U_LEFT, {
			std::cout << "INFO: User left current group (uid: " << read32(body) << ")" << std::endl;
		});

	}
}

