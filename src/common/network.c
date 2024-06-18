
#include "network.h"

int net_write(NioFunctor* base, NioStream* stream, void* buffer, int bytes) {

	const int total = bytes;

	while (bytes != 0) {
		const int result = base->write(stream, buffer, bytes);

		if (result == -1 || result == 0) {
			return result;
		}

		bytes -= result;
		buffer += result;
	}

	return total;

}

int net_read(NioFunctor* base, NioStream* stream, void* buffer, int bytes) {

	const int total = bytes;

	while (bytes != 0) {
		const int result = base->read(stream, buffer, bytes);

		if (result == -1 || result == 0) {
			return result;
		}

		bytes -= result;
		buffer += result;
	}

	return total;

}
