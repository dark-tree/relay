
#include "writer.hpp"

void Packet::send(tcp::socket& sock) const {
	asio::write(sock, asio::buffer(data, data.size()));
}

PacketWriter& PacketWriter::write32(uint32_t value) {
	size_t size = data.size();
	data.resize(size + 4, 0);
	memcpy(data.data() + size, &value, 4);

	return *this;
}

PacketWriter& PacketWriter::write16(uint16_t value) {
	size_t size = data.size();
	data.resize(size + 2, 0);
	memcpy(data.data() + size, &value, 2);

	return *this;
}

PacketWriter& PacketWriter::write8(uint8_t value) {
	data.push_back(value);

	return *this;
}

PacketWriter& PacketWriter::write(uint8_t* buffer, uint16_t length) {
	size_t size = data.size();
	data.resize(size + length, 0);
	memcpy(data.data() + size, buffer, length);

	return *this;
}

PacketWriter& PacketWriter::write(std::string str) {
	write((uint8_t*) str.c_str(), str.size());

	return *this;
}

Packet PacketWriter::pack() {
	uint16_t size = data.size() - 3;
	memcpy(data.data() + 1, &size, 2);

	return Packet(std::move(data));
}

