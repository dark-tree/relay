
/*
 *  Copyright (C) 2024 magistermaks
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
