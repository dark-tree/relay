
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

#include "external.h"

#include <client/strings.h>
#include <common/const.h>
#include <common/stream.h>
#include <common/input.h>
#include <common/logger.h>
#include <common/util.h>
#include <common/network.h>

sem_t fsem;
FILE* fptr = NULL;
int fuid = 0;

void* server_listener(void* user) {
	NioStream* stream = (NioStream*) user;

	struct timeval initial_read;
	struct timeval default_read;

	util_mstime(&initial_read, 100);
	util_mstime(&default_read, 0);

	while (nio_open(stream)) {

		uint8_t id = 0;

		nio_timeout(stream, &initial_read);

		while (true) {

			if (nio_header(stream, &id)) {
				break;
			}

		}

		nio_timeout(stream, &default_read);

		if (!nio_open(stream)) {
			break;
		}

		if (id == R2U_WELC) {

			uint16_t ver = nio_read16(stream);
			uint16_t rev = nio_read16(stream);
			uint32_t uid = nio_read32(stream);

			uint8_t buffer[65];
			nio_read(stream, buffer, 64);
			buffer[64] = 0;

			// remove any sneaky characters that could brake formatting
			util_sanitize(buffer, 64);

			log_info("Recieved URP welcome, using protocol v%d.%d (uid: %d)\n", ver, rev, uid);
			log_info("Server identifies as: '%s'\n", buffer);
			continue;
		}

		if (id == R2U_TEXT) {
			uint32_t uid = nio_read32(stream);
			uint32_t len = nio_read32(stream);

			uint8_t buffer[len + 1];
			nio_read(stream, buffer, len);
			buffer[len] = 0;

			if (fptr && uid == fuid) {

				sem_wait(&fsem);
				fwrite(buffer, 1, len, fptr);
				fclose(fptr);
				fptr = 0;
				fuid = 0;
				sem_post(&fsem);

				log_info("User #%d message saved to file\n", uid);
				continue;
			}

			util_sanitize(buffer, len);

			log_info("User #%d said: '%s'\n", uid, buffer);
			continue;
		}

		if (id == R2U_MADE) {
			uint16_t sta = nio_read8(stream);
			uint32_t gid = nio_read32(stream);

			log_info(str_makes_decode(sta), gid);
			continue;
		}

		if (id == R2U_STAT) {
			uint16_t sta = nio_read8(stream);
			log_info("Your status changed to: %s\n", str_role_decode(sta));
			continue;
		}

		if (id == R2U_JOIN) {
			uint32_t uid = nio_read32(stream);
			log_info("User #%d joined\n", uid);
			continue;
		}

		if (id == R2U_LEFT) {
			uint32_t uid = nio_read32(stream);
			log_info("User #%d left\n", uid);
			continue;
		}

		if (id == R2U_VALS) {
			uint32_t key = nio_read32(stream);
			uint32_t val = nio_read32(stream);

			log_info("Setting '%s' is set to '%d'\n", str_sets_decode(key), val);
		}

	}

	log_info("Connection closed\n");

	nio_free(stream);
	exit(0);

}

int main(int argc, char* argv[]) {

	if (argc != 2) {
		printf("Usage: client [hostname]\n");
		exit(1);
	}

	struct hostent* host = gethostbyname(argv[1]);

	if (host == NULL) {
		log_fatal("Failed to decode hostname!\n");
		exit(-1);
	}

	struct sockaddr_in address;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		log_fatal("Failed to open socket!\n");
		exit(-1);
	}

	address.sin_family = AF_INET;
	address.sin_port = htons(9686);
	memcpy(&address.sin_addr, host->h_addr_list[0], host->h_length);

	if (connect(sockfd, (struct sockaddr*) &address, sizeof(address)) != 0) {
		log_fatal("Failed to connect to server!\n");
		exit(-1);
	}

	log_info("Successfully made a TCP connection to server.\n");

	NetConsts consts = {0};
	consts.connfd = sockfd;

	NioStream stream;
	nio_create(&stream, 1024, net_raw(&consts));

	sem_init(&fsem, 0, 1);
	pthread_t thread;
	pthread_create(&thread, NULL, server_listener, &stream);

	InputLine line;
	line.line = NULL;
	line.length = 0;

	char buffer[255];

	while (true) {
		input_readline(&line);

		if (input_token(&line, buffer, 255)) {

			if (streq(buffer, "help")) {
				printf("List of commands:\n");
				printf(" * help                   Print this help page\n");
				printf(" * push <byte>            Write one byte into the server connection\n");
				printf(" * make                   Create new group\n");
				printf(" * join <gid> <password>  Join new group\n");
				printf(" * quit                   Quit the group\n");
				printf(" * kick <uid>             Kick user from group\n");
				printf(" * send <uid> <message>   Send message to user\n");
				printf(" * brod <uid> <message>   Send message to all, except user\n");
				printf(" * rand <count>           Write 'count' random bytes into the server connection\n");
				printf(" * stop                   Stop the client\n");
				printf(" * gets <key>             Get the value of a setting\n");
				printf(" * sets <key> <value>     Set the value of a setting\n");
				printf(" * file r/w <uid> <path>  Send file as message or collect next message to file\n");
			}

			if (streq(buffer, "push")) {

				long value;
				if (input_number(&line, &value)) {
					nio_write8(&stream, value);
				}

			}

			if (streq(buffer, "rand")) {

				long value;
				if (input_number(&line, &value)) {

					for (int i = 0; i < value; i ++) {
						nio_write8(&stream, rand());
					}

				}

			}

			if (streq(buffer, "make")) {
				nio_write8(&stream, U2R_MAKE);
			}

			if (streq(buffer, "join")) {

				long gid;
				if (input_number(&line, &gid)) {

					long pass;
					if (input_number(&line, &pass)) {

						nio_write8(&stream, U2R_JOIN);
						nio_write32(&stream, gid);
						nio_write32(&stream, pass);

					}

				}

			}

			if (streq(buffer, "quit")) {
				nio_write8(&stream, U2R_QUIT);
			}

			if (streq(buffer, "stop")) {
				nio_drop(&stream);
			}

			if (streq(buffer, "kick")) {

				long uid;
				if (input_number(&line, &uid)) {

					nio_write8(&stream, U2R_KICK);
					nio_write32(&stream, uid);

				}

			}

			if (streq(buffer, "send")) {

				long uid;
				if (input_number(&line, &uid)) {

					if (input_string(&line, buffer, 255)) {

						long len = strlen(buffer);

						nio_write8(&stream, U2R_SEND);
						nio_write32(&stream, uid);
						nio_write32(&stream, len);
						nio_write(&stream, buffer, len);

					}

				}

			}

			if (streq(buffer, "brod")) {

				long uid;
				if (input_number(&line, &uid)) {

					if (input_string(&line, buffer, 255)) {

						long len = strlen(buffer);

						nio_write8(&stream, U2R_BROD);
						nio_write32(&stream, uid);
						nio_write32(&stream, len);
						nio_write(&stream, buffer, len);

					}

				}

			}

			if (streq(buffer, "gets")) {
				if (input_string(&line, buffer, 255)) {

					uint32_t key = str_sets_encode(buffer);

					nio_write8(&stream, U2R_GETS);
					nio_write32(&stream, key);

				}
			}

			if (streq(buffer, "sets")) {
				if (input_string(&line, buffer, 255)) {

					uint32_t key = str_sets_encode(buffer);

					long val;
					if (input_number(&line, &val)) {

						nio_write8(&stream, U2R_SETS);
						nio_write32(&stream, key);
						nio_write32(&stream, val);

					}

				}
			}

			if (streq(buffer, "file")) {
				if (input_token(&line, buffer, 255)) {

					int mode = 0;

					if (streq(buffer, "r")) {
						mode = 1;
					}

					if (streq(buffer, "w")) {
						mode = 2;
					}

					if (mode) {

						long uid;
						if (input_number(&line, &uid)) {
							if (input_string(&line, buffer, 255)) {

								if (mode == 1) { // read - send the given file as message
									sem_wait(&fsem);
									FILE* f = fopen(buffer, "r");

									if (f) {

										fseek(f, 0, SEEK_END);
										long length = ftell(f);
										fseek(f, 0, SEEK_SET);

										uint8_t buffer[length];
										fread(buffer, 1, length, f);

										nio_write8(&stream, U2R_SEND);
										nio_write32(&stream, uid);
										nio_write32(&stream, length);
										nio_write(&stream, buffer, length);

									} else {
										log_warn("Filed to open file\n");
									}
									sem_post(&fsem);
								}

								if (mode == 2) { // write - save next message from UID to file
									sem_wait(&fsem);

									if (fptr) {
										log_warn("Another file was already scheduled and was replaced\n");
										fclose(fptr);
									}

									fptr = fopen(buffer, "w");

									if (!fptr) {
										log_warn("Filed to open file\n");
									}

									fuid = uid;
									sem_post(&fsem);
								}

							}
						}

					}
				}
			}

		}
	}

	input_free(&line);
	sem_destroy(&fsem);

}
