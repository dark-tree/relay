
#pragma once

#include "core.hpp"

class __attribute__((__packed__)) PacketHead {

	protected:

		uint32_t read32(uint8_t* data) {
			return *((uint32_t*) data);
		}

		uint16_t read16(uint8_t* data) {
			return *((uint16_t*) data);
		}

		uint8_t read8(uint8_t* data) {
			return *data;
		}

	public:

		const uint8_t type;
		const uint16_t size;

		PacketHead() 
		: type(0), size(0) {
			// dummy implementation, deserialize with memcpy
		}

};

