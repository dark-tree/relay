
#pragma once

#include "core.hpp"

class __attribute__((__packed__)) PacketHead {

	protected:
		uint32_t read32(uint8_t* data) {
			uint32_t val;
			memcpy((uint8_t*) &val, data, 4);
			return val;
		}

	public:
		const uint8_t type;
		const uint16_t size;

		PacketHead() : type(0), size(0) {
			// dummy implementation, deserialize with memcpy
		}

};

