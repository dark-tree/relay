
#pragma once

#include "core.hpp"

class Packet {

	private:

		std::vector<uint8_t> data;

	public:

		Packet(std::vector<uint8_t> data) : data(std::move(data)) {
			// move vector
		}

		/// write the packet to a socket
		void send(tcp::socket& sock) const;

};

class PacketWriter {

	private:

		const uint8_t type;
		std::vector<uint8_t> data;

	public:

		PacketWriter(uint8_t type) : type(type) {
			data.resize(3, 0);
			data[0] = type;
		}

		/// add a single 32bit unsigned integer into the packet
		PacketWriter& write(uint32_t value);

		/// add a buffer of given size into the packet
		PacketWriter& write(uint8_t* buffer, uint16_t length);

		/// add a string into the packet
		PacketWriter& write(std::string str);

		/// get the packet ready for sending
		Packet pack();

};

