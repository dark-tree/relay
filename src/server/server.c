
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>

#include <common/logger.h>
#include <common/map.h>
#include <common/const.h>
#include <common/vec.h>
#include <common/input.h>
#include <server/mutex.h>
#include <server/user.h>
#include <server/group.h>
#include <server/sequence.h>

// And here truth was made, and disjointed chaos of the threads
// synchronized into one so that a singular state could emerge,
// and God saw that it was good.
sem_t truth_mutex;

IdMap* users;
SharedMutex user_mutex;
IdSequence uid_sequence;

IdMap* groups;
SharedMutex group_mutex;
IdSequence gid_sequence;

void accept_handle(int sock, int conn);

void* tcps_accept(void* user) {

	int sockfd = (long) user;

	while (true) {

		struct sockaddr_in address;
		int len = sizeof(address);

		int connfd = accept(sockfd, (struct sockaddr*) &address, &len);

		if (connfd < 0) {
			log_warn("Failed to accept connection!\n");
			continue;
		}

		accept_handle(sockfd, connfd);

	}

	close(sockfd);

}

void tcps_start(uint16_t port, uint16_t backlog, void (*accept_callback) (int sockfd, int connfd)) {

	struct sockaddr_in address;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		log_fatal("Failed to open socket!\n");
		exit(-1);
	}

	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		log_error("Failed to configure socket address!\n");
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
		log_error("Failed to configure socket port!\n");
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
		log_error("Failed to configure socket port!\n");
	}

	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, (struct sockaddr*) &address, sizeof(address)) != 0) {
		log_fatal("Failed to bind to port!\n");
		exit(-1);
	}

	if (listen(sockfd, backlog) != 0) {
		log_fatal("Failed to start listening on socket!\n");
		exit(-1);
	}

	pthread_t thread;
	pthread_create(&thread, NULL, tcps_accept, (void*) (long) sockfd);

}

void milis(uint32_t msec, struct timeval *tv) {
	tv->tv_sec = msec / 1000;
	tv->tv_usec = (msec % 1000) * 1000;
}

void* user_thread(void* context) {
	User * const user = (User*) context;
	NioStream* stream = &user->stream;

	log_info("User #%d connected\n", user->uid);

	uint8_t brand[64] = "My Little Relay - Ethernet To Magia!";

	NIO_CORK(stream, {
		// send the welcome packet
		nio_write8(stream, R2U_WELC);
		nio_write16(stream, URP_VERSION);
		nio_write16(stream, URP_REVISION);
		nio_write32(stream, user->uid);
		nio_write(stream, brand, 64);

		// send inital status packet
		nio_write8(stream, R2U_STAT);
		nio_write8(stream, ROLE_CONNECTED);
	});

	struct timeval initial_read;
	struct timeval kupa;

	milis(50, &initial_read);
	milis(0, &kupa);

	while (nio_open(stream)) {

		uint8_t id;

		nio_timeout(stream, &initial_read);

		while (true) {

			// This check is thread safe
			// The only thing we check here is if we joined the group
			// not if the group was deleted, as that cannot happen on its own.
			if (user->group && (user->exit || user->group->close)) {

				const uint32_t gid = user->group->gid;
				group_exit(user->group, user);

				if (user->exit) {
					log_info("User #%d was kicked from group #%d\n", user->uid, gid);
				} else {
					log_info("User #%d left group #%d as it is being disbanded\n", user->uid, gid);
				}

				user->exit = false;

			}

			// try reading the packet header, returns 0 if timeout occures
			if (nio_header(stream, &id)) {
				break;
			}

		}

		nio_timeout(stream, &kupa);

		if (id == U2R_MAKE) {

			if (user->role != ROLE_CONNECTED) {
				continue;
			}

			const uint32_t gid = idseq_next(&gid_sequence);
			Group* group = group_create(gid, user);

			UNIQUE_LOCK(&group_mutex, {
				idmap_put(groups, gid, group);
			});

			user->role = ROLE_HOST;
			user->group = group;

			// TODO write_mutex?
			nio_write8(stream, R2U_MADE);
			nio_write8(stream, 1);
			nio_write32(stream, gid);

			nio_write8(stream, R2U_STAT);
			nio_write8(stream, ROLE_HOST);

			log_info("User #%d created group #%d\n", user->uid, gid);

			continue;
		}

		if (id == U2R_JOIN) {

			const uint32_t gid = nio_read32(stream);
			const uint32_t pass = nio_read32(stream);

			if (user->role != ROLE_CONNECTED) {
				// TODO: spec
				continue;
			}

			// this lock serves a dual purpose: it grants read-only access to the group hashmap
			// and guarantees that the group will not be deleted mid-usage. We do not yet hold
			// a reference to the group, so it can be deleted by the host if not for the cleanup thread
			// requiring a unique lock on the same mutex to finish the removal process.
			SHARED_LOCK(&group_mutex, {

				Group* const group = idmap_get(groups, gid);

				if (!group || group->close) {

					// TODO: spec
					SEMAPHORE_LOCK(&user->write_mutex, {
						nio_write8(stream, R2U_MADE);
						nio_write8(stream, 0);
						nio_write32(stream, 0);
					});

				} else {

					// wait for master group lock as
					// we need to modify the group member list
					SEMAPHORE_LOCK(&group->master_mutex, {

						// verify if the group is really not closing
						// and if the host is still in it
						if (group->host && !group->close) {

							// add new member
							idvec_put(&group->members, user);

							group->refcnt ++;
							user->role = ROLE_MEMBER;
							user->group = group;

							// notify group host
							SEMAPHORE_LOCK(&group->host->write_mutex, {
								nio_write8(&group->host->stream, R2U_JOIN);
								nio_write32(&group->host->stream, user->uid);
							});

							// notify new member
							SEMAPHORE_LOCK(&user->write_mutex, {
								nio_write8(stream, R2U_MADE);
								nio_write8(stream, 2);
								nio_write32(stream, group->gid);

								nio_write8(stream, R2U_STAT);
								nio_write8(stream, ROLE_MEMBER);
							});

							log_info("User #%d joined group #%d\n", user->uid, group->gid);

						} else {

							// TODO: spec
							SEMAPHORE_LOCK(&user->write_mutex, {
								nio_write8(stream, R2U_MADE);
								nio_write8(stream, 0);
								nio_write32(stream, 0);
							});

						}

					});

				}

			});

			continue;
		}

		if (id == U2R_QUIT) {

			if (user->role == ROLE_CONNECTED) {
				continue;
			}

			Group* group = user->group;

			const uint32_t uid = user->uid;
			const uint32_t gid = user->group->gid;

			if (group->uid == uid) {
				group_disband(group);
				continue;
			}

			group_exit(group, user);
			log_info("User #%d left group #%d\n", uid, gid);

			continue;
		}

		if (id == U2R_KICK) {

			const uint32_t uid = nio_read32(stream);

			if (user->role != ROLE_HOST) {
				continue;
			}

			Group* group = user->group;

			const uint32_t gid = user->group->gid;

			if (group->uid == uid) {
				group_disband(group);
				continue;
			}

			if (!group_remove(group, uid)) {
				log_warn("User #%d tried to kick unassociated or non-existant user #%d", user->uid, uid);
			}

			continue;
		}

		if (id == U2R_SEND) {

			const uint32_t uid = nio_read32(stream);
			const uint32_t len = nio_read32(stream);

			// packet was send while user was in invalid state
			// we have to read the message bytes for the stream
			// to stay synchronized
			if (user->role == ROLE_CONNECTED) {
				nio_skip(stream, len);
				continue;
			}

			// TODO potentially optimize user access
			SHARED_LOCK(&user_mutex, {

				User* target = idmap_get(users, uid);

				if (target && target->group == user->group) {

					// TODO read first block before we lock target

					SEMAPHORE_LOCK(&target->write_mutex, {

						NIO_CORK(&target->stream, {

							// send header once
							nio_write8(&target->stream, R2U_TEXT);
							nio_write32(&target->stream, user->uid);
							nio_write32(&target->stream, len);

							NioBlock block = nio_block(stream, len);

							while (block.remaining) {

								// TODO add guard byte
								// TODO packet interweaving

								nio_readbuf(stream, &block);
								nio_writebuf(&target->stream, &block);

							}

						});

					});

				} else {

					// there is no such user with the given uid, or the user
					// is not in our group, but as before we need to read the message anyway
					nio_skip(stream, len);

				}

			});

			continue;

		}

		if (id == U2R_BROD) {

			const uint32_t uid = nio_read32(stream);
			const uint32_t len = nio_read32(stream);

			// packet send while user was in invalid state
			// we have to read the message bytes for the stream
			// to stay synchronized
			if (user->role == ROLE_CONNECTED) {
				nio_skip(stream, len);
				continue;
			}

			Group* group = user->group;

			// wait for master group lock as
			// we will lock more than one user lock
			// and need the group to be constant
			SEMAPHORE_LOCK(&group->master_mutex, {

				NioBlock block = nio_block(stream, len);

				// lock targets
				IDVEC_FOREACH(User*, target, group->members) {
					if (target->uid != uid) {
						sem_wait(&target->write_mutex);
						nio_cork(&target->stream, true);

						// send header once
						nio_write8(&target->stream, R2U_TEXT);
						nio_write32(&target->stream, user->uid);
						nio_write32(&target->stream, len);
					}
				}

				while (block.remaining) {

					// TODO add guard byte
					// TODO packet interweaving

					nio_readbuf(stream, &block);

					IDVEC_FOREACH(User*, target, group->members) {
						if (target->uid != uid) {
							nio_writebuf(&target->stream, &block);
						}
					}

				}

				// unlock targets
				IDVEC_FOREACH(User*, unlock, group->members) {
					if (unlock->uid != uid) {
						nio_cork(&unlock->stream, false);
						sem_post(&unlock->write_mutex);
					}
				}

			});

		}

	}

	// TODO maybe move it to some function? idk
	// copy U2R_QUIT logic
	if (user->role != ROLE_CONNECTED) {

		Group* group = user->group;

		const uint32_t uid = user->uid;
		const uint32_t gid = user->group->gid;

		if (group->uid == uid) {
			group_disband(group);
		} else {
			group_exit(group, user);
			log_info("User #%d left group #%d\n", uid, gid);
		}

	}

	UNIQUE_LOCK(&user_mutex, {
		idmap_remove(users, user->uid);
	});

	log_info("User #%d disconnected\n", user->uid);

	user_free(user);
}

void accept_handle(int sock, int conn) {

	int uid = idseq_next(&uid_sequence);
	User* user = user_create(uid, conn);

	UNIQUE_LOCK(&user_mutex, {
		idmap_put(users, uid, user);
	});

	pthread_t thread;
	pthread_create(&thread, NULL, user_thread, user);
}

int main() {

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		log_fatal("Failed to set a ignore handler for SIGPIPE!\n");
		exit(-1);
	}

	users = idmap_create();
	groups = idmap_create();

	mutex_init(&user_mutex);
	mutex_init(&group_mutex);
	sem_init(&truth_mutex, 0, 1);

	idseq_begin(&uid_sequence, IDSEQ_MONOTONIC);
	idseq_begin(&gid_sequence, IDSEQ_MONOTONIC);

	tcps_start(9686, 8, accept_handle);

	InputLine line;
	line.line = NULL;
	line.length = 0;

	char buffer[255];

	while (true) {
		input_readline(&line);

		if (!input_token(&line, buffer, 255, false)) {
			continue;
		}

		if (streq(buffer, "help")) {
			printf("List of commands:\n");
			printf(" * help           Print this help page\n");
			printf(" * users          List all users\n");
			printf(" * groups         List all groups\n");
			printf(" * members <gid>  List all members of a group\n");
		}

		if (streq(buffer, "users")) {
			printf("List of users:\n");

			SHARED_LOCK(&user_mutex, {
				IdMapIterator iter = idmap_iterator(users);

				while (idmap_has(&iter)) {
					User* user = (User*) idmap_next(&iter);

					if (user->role == ROLE_CONNECTED) {
						printf(" * user #%d connected\n", user->uid);
						continue;
					}

					if (user->role == ROLE_MEMBER) {
						printf(" * user #%d member of group #%d\n", user->uid, user->group->gid); // TODO unsafe
						continue;
					}

					if (user->role == ROLE_HOST) {
						printf(" * user #%d host of group #%d\n", user->uid, user->group->gid); // TODO unsafe
						continue;
					}
				}
			});
		}

		if (streq(buffer, "groups")) {
			printf("List of groups:\n");

			SHARED_LOCK(&group_mutex, {
				IdMapIterator iter = idmap_iterator(groups);

				while (idmap_has(&iter)) {
					Group* group = (Group*) idmap_next(&iter);

					printf(" * group #%d with host #%d\n", group->gid, group->uid);
				}
			});
		}

		if (streq(buffer, "members")) {
			long gid;
			if (input_number(&line, &gid)) {
				printf("List of members:\n");

				SHARED_LOCK(&group_mutex, {

					Group* group = idmap_get(groups, gid);

					if (!group) {
						printf("The group with the given GID does not exist!\n");
					} else {

						SEMAPHORE_LOCK(&group->master_mutex, { // TODO unsafe

							IDVEC_FOREACH(User*, user, group->members) {
								printf(" * user #%d\n", user->uid);
							}

						});

					}

				});
			}
		}

	}

	input_free(&line);

}
