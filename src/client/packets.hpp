
#pragma once

#include "head.hpp"
#include "core.hpp"

class __attribute__((__packed__)) ClientPacketHead : public PacketHead {

	public:

		/// process the packet body
		void accept(uint8_t* body);

};

