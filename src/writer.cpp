
#include "writer.hpp"

void Packet::send(tcp::socket& sock) const {
	asio::write(sock, asio::buffer(data, data.size()));
}

PacketWriter& PacketWriter::write(uint32_t value) {
	size_t size = data.size();
	data.resize(size + 4, 0);
	memcpy(data.data() + size, &value, 4);

	return *this;
}

PacketWriter& PacketWriter::write(uint8_t* buffer, uint16_t length) {
	size_t size = data.size();
	data.resize(size + length, 0);
	memcpy(data.data() + size, buffer, length);

	return *this;
}

Packet PacketWriter::pack() {
	uint16_t size = data.size() - 3;
	memcpy(data.data() + 1, &size, 2);

	return Packet(std::move(data));
}

