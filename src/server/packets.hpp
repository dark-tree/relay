
#pragma once

#include "head.hpp"
#include "users.hpp"
#include "core.hpp"

class __attribute__((__packed__)) ServerPacketHead : public PacketHead {

	public:

		/// process the packet body
		void accept(uint8_t* body, std::shared_ptr<User> user);

};

