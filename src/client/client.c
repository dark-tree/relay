
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#include "common/const.h"
#include "common/stream.h"
#include "common/input.h"

// TODO move somewhere else
const char* made_codestr(uint8_t code) {
	if (code == 0x01) return "Created group #%d\n";
	if (code == 0x02) return "Joined group #%d\n";

	return "Failed to joined the given group!\n";
}

// TODO move somewhere else
const char* role_tostr(uint8_t role) {
	if (role == 1) return "connected";
	if (role == 2) return "member";
	if (role == 4) return "host";

	return "<invalid value>";
}

void sanitize_buffer(uint8_t* buffer, uint32_t len) {

	for (int i = 0; i < len; i ++) {
		uint8_t byte = buffer[i];

		if (byte == 0) {
			return;
		}

		if (byte < ' ') {
			buffer[i] = '?';
		}

		if (byte > '~') {
			buffer[i] = '?';
		}
	}

}

void* server_listener(void* user) {
	NioStream* stream = (NioStream*) user;

	while (nio_open(stream)) {

		uint8_t id = nio_read8(stream);

		if (id == R2U_WELC) {

			uint16_t ver = nio_read16(stream);
			uint16_t rev = nio_read16(stream);
			uint32_t uid = nio_read32(stream);

			uint8_t buffer[65];
			nio_read(stream, buffer, 64);
			buffer[64] = 0;

			// remove any sneaky characters that could brake formatting
			sanitize_buffer(buffer, 64);

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

			sanitize_buffer(buffer, len);

			log_info("User #%d said: '%s'\n", uid, buffer);
			continue;
		}

		if (id == R2U_MADE) {
			uint16_t sta = nio_read8(stream);
			uint32_t gid = nio_read32(stream);

			log_info(made_codestr(sta), gid);
			continue;
		}

		if (id == R2U_STAT) {
			uint16_t sta = nio_read8(stream);
			log_info("Your status changed to: %s\n", role_tostr(sta));
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

	}

	log_info("Connection closed\n");
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
	address.sin_port = htons(URP_PORT);
	memcpy(&address.sin_addr, host->h_addr_list[0], host->h_length);

	if (connect(sockfd, (struct sockaddr*) &address, sizeof(address)) != 0) {
        log_fatal("Failed to connect to server!\n");
        exit(-1);
    }

	log_info("Successfully made a TCP connection to server.\n");

	NioStream stream;
	nio_create(&stream, (int) (uint64_t) sockfd, 1024);

	pthread_t thread;
	pthread_create(&thread, NULL, server_listener, &stream);

	InputLine line;
	line.line = NULL;
	line.length = 0;

	char buffer[255];

	while (true) {
		input_readline(&line);

		if (input_token(&line, buffer, 255, false)) {

			if (streq(buffer, "help")) {
				printf("List of commands:\n");
				printf(" * help               Print this help page\n");
				printf(" * push <byte>        Write one byte into the server connection\n");
				printf(" * make               Create new group\n");
				printf(" * join <gid> <pass>  Join new group\n");
				printf(" * quit               Quit the group\n");
				printf(" * kick <uid>         Kick user from group\n");
				printf(" * send <uid> <msg>   Send message to user\n");
				printf(" * brod <uid> <msg>   Send message to all, except user\n");
			}

			if (streq(buffer, "push")) {

				long value;
				if (input_number(&line, &value)) {
					nio_write8(&stream, value);
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

					if (input_token(&line, buffer, 255, true)) {

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

					if (input_token(&line, buffer, 255, true)) {

						long len = strlen(buffer);

						nio_write8(&stream, U2R_BROD);
						nio_write32(&stream, uid);
						nio_write32(&stream, len);
						nio_write(&stream, buffer, len);

					}

				}

			}

		}
	}

	input_free(&line);

}
