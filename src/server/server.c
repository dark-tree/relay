
#include "external.h"

#include <common/logger.h>
#include <common/map.h>
#include <common/const.h>
#include <common/vec.h>
#include <common/input.h>
#include <common/util.h>
#include <server/mutex.h>
#include <server/user.h>
#include <server/group.h>
#include <server/sequence.h>
#include <server/tcps.h>
#include <server/store.h>

// And here truth was made, and disjointed chaos of the threads
// synchronized into one so that a singular state could emerge,
// and God saw that it was good.
// sem_t truth_mutex;

IdStore* users;
IdStore* groups;

void* user_thread(void* context) {

	User * const user = (User*) context;
	NioStream* stream = &user->stream;

	log_info("User #%d connected\n", user->uid);

	uint8_t brand[64] = "My Little Relay";

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
	struct timeval default_read;

	util_mstime(&initial_read, 50);
	util_mstime(&default_read, 100);

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

		// if not for this check, when nio_header fails to read data (which is higly likely)
		// the server would erroniusly execute one extra packet
		if (!nio_open(stream)) {
			break;
		}

		nio_timeout(stream, &default_read);

		if (id == U2R_MAKE) {

			log_debug("Received from user #%d: U2R_MAKE {}\n", user->uid);

			if (user_verify(user, ROLE_CONNECTED)) {
				continue;
			}


			Group* group = store_putgroup(groups, user);

			user->role = ROLE_HOST;
			user->group = group;

			// TODO consider the use of this mutex
			SEMAPHORE_LOCK(&user->write_mutex, {
				nio_write8(stream, R2U_MADE);
				nio_write8(stream, FROM_MAKE | STAT_OK);
				nio_write32(stream, group->gid);

				nio_write8(stream, R2U_STAT);
				nio_write8(stream, ROLE_HOST);
			});

			log_info("User #%d created group #%d\n", user->uid, group->gid);

			continue;
		}

		if (id == U2R_JOIN) {

			const uint32_t gid = nio_read32(stream);
			const uint32_t pass = nio_read32(stream);

			log_debug("Received from user #%d: U2R_JOIN {gid=%u, pass=%u}\n", user->uid, gid, pass);

			if (user_verify(user, ROLE_CONNECTED)) {
				continue;
			}

			// this lock serves a dual purpose: it grants read-only access to the group hashmap
			// and guarantees that the group will not be deleted mid-usage. We do not yet hold
			// a reference to the group, so it can be deleted by the host if not for the cleanup thread
			// requiring a unique lock on the same mutex to finish the removal process.
			SHARED_LOCK(&groups->mutex, {

				const uint8_t status_closed = FROM_JOIN | STAT_ERROR_INVALID;
				Group* const group = idmap_get(groups->map, gid);

				if (!group || group->close) {

					SEMAPHORE_LOCK(&user->write_mutex, {
						nio_write8(stream, R2U_MADE);
						nio_write8(stream, status_closed);
						nio_write32(stream, NULL_GROUP);
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
								nio_write8(stream, FROM_JOIN | STAT_OK);
								nio_write32(stream, group->gid);

								nio_write8(stream, R2U_STAT);
								nio_write8(stream, ROLE_MEMBER);
							});

							log_info("User #%d joined group #%d\n", user->uid, group->gid);

						} else {

							SEMAPHORE_LOCK(&user->write_mutex, {
								nio_write8(stream, R2U_MADE);
								nio_write8(stream, status_closed);
								nio_write32(stream, NULL_GROUP);
							});

						}

					});

				}

			});

			continue;
		}

		if (id == U2R_QUIT) {

			log_debug("Received from user #%d: U2R_QUIT {}\n", user->uid);

			if (user_verify(user, ROLE_HOST | ROLE_MEMBER)) {
				continue;
			}

			user_quit(user);
			continue;
		}

		if (id == U2R_KICK) {

			const uint32_t uid = nio_read32(stream);

			log_debug("Received from user #%d: U2R_KICK {uid=%u}\n", user->uid, uid);

			if (user_verify(user, ROLE_HOST)) {
				continue;
			}

			user_kick(user, uid);
			continue;
		}

		if (id == U2R_SEND) {

			const uint32_t uid = nio_read32(stream);
			const uint32_t len = nio_read32(stream);

			log_debug("Received from user #%d: U2R_SEND {uid=%u, len=%u}\n", user->uid, uid, len);

			if (user_verify(user, ROLE_HOST | ROLE_MEMBER)) {

				// packet was send while user was in invalid state
				// we have to read the message bytes from the stream
				// to stay synchronized
				nio_skip(stream, len);

				continue;
			}

			// TODO opti: potentially optimize user access
			SHARED_LOCK(&users->mutex, {

				User* target = idmap_get(users->map, uid);

				if (target && target->group == user->group) {

					// TODO opti: read first block before we lock target
					SEMAPHORE_LOCK(&target->write_mutex, {

						NIO_CORK(&target->stream, {

							// send header once
							nio_write8(&target->stream, R2U_TEXT);
							nio_write32(&target->stream, user->uid);
							nio_write32(&target->stream, len);

							NioBlock block = nio_block(stream, len);

							while (block.remaining) {

								// TODO spec: add guard byte at every section (allowing the packet to be ended early)
								// TODO opti: packet interweaving

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

			log_debug("Received from user #%d: U2R_BROD {uid=%u, len=%u}\n", user->uid, uid, len);

			// packet send while user was in invalid state
			// we have to read the message bytes for the stream
			// to stay synchronized
			if (user->role == ROLE_CONNECTED) {

				nio_skip(stream, len);

				SEMAPHORE_LOCK(&user->write_mutex, {
					nio_write8(stream, R2U_STAT);
					nio_write8(stream, user->role);
				});

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

					// TODO spac: add guard byte at every section (allowing the packet to be ended early)
					// TODO opti: packet interweaving

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

		if (id == U2R_SETS) {

			uint8_t notify = nio_read8(stream);
			uint32_t key = nio_read32(stream);
			uint32_t var = nio_read32(stream);

			uint32_t* settings = user_setting(user, key, true);

			if (!settings) {

				SEMAPHORE_LOCK(&user->write_mutex, {
					nio_write8(stream, R2U_VALS);
					nio_write32(stream, SETK_INVALID);
					nio_write32(stream, key);
				});

			}

			continue;
		}

		if (id == U2R_GETS) {

			uint32_t key = nio_read32(stream);

			uint32_t* setting = user_setting(user, key, false);

			SEMAPHORE_LOCK(&user->write_mutex, {
				NIO_CORK(stream, {
					nio_write8(stream, R2U_VALS);

					if (setting) {
						nio_write32(stream, key);
						nio_write32(stream, *setting);
					} else {

						// if there is no setting with the requested key
						// respond back with a invalid setting key
						nio_write32(stream, SETK_INVALID);
						nio_write32(stream, key);

					}
				});
			});

			continue;
		}

	}

	// same logic as the U2R_QUIT packet
	if (user->role != ROLE_CONNECTED) {
		user_quit(user);
	}

	store_remove(users, user->uid);
	log_info("User #%d disconnected\n", user->uid);

	user_free(user);
}

void accept_handle(int conn) {

	User* user = store_putuser(users, conn);

	pthread_t thread;
	pthread_create(&thread, NULL, user_thread, user);
}

void cleanup_handle(int sock) {

	SHARED_LOCK(&users->mutex, {
		IdMapIterator iter = idmap_iterator(users->map);

		while (idmap_has(&iter)) {
			nio_drop(&(((User*) idmap_next(&iter))->stream));
		}
	});

	// spin lock!
	while (users->counter > 0) {
		usleep(10000); // 10ms
	}

	close(sock);
	store_free(users);
	store_free(groups);
	exit(0);

}

int main() {

	// we need to block it so that write() doesn't trigger signals on error
	// as that would make it harder for use to detect errors
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		log_fatal("Failed to set a ignore handler for SIGPIPE!\n");
		exit(-1);
	}

	users = store_create(IDSEQ_MONOTONIC);
	groups = store_create(IDSEQ_MONOTONIC);

	TcpServer server;
	server.accept_callback = accept_handle;
	server.cleanup_callback = cleanup_handle;

	tcps_start(&server, 9686, 8);

	InputLine line;
	line.line = NULL;
	line.length = 0;

	char buffer[255];

	while (true) {
		input_readline(&line);

		if (!input_token(&line, buffer, 255)) {
			continue;
		}

		if (streq(buffer, "help")) {
			printf("List of commands:\n");
			printf(" * help           Print this help page\n");
			printf(" * users          List all users\n");
			printf(" * groups         List all groups\n");
			printf(" * members <gid>  List all members of a group\n");
			printf(" * stop           Stop the server\n");
		}

		if (streq(buffer, "stop")) {
			tcps_stop(&server);
		}

		if (streq(buffer, "users")) {

			if (users->counter == 0) {
				printf("There are currently no connected users.\n");
				continue;
			}

			printf("List of all %d users:\n", users->counter);

			SHARED_LOCK(&users->mutex, {
				IdMapIterator iter = idmap_iterator(users->map);

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

			if (groups->counter == 0) {
				printf("There are currently no open groups.\n");
				continue;
			}

			printf("List of all %d groups:\n", groups->counter);

			SHARED_LOCK(&groups->mutex, {
				IdMapIterator iter = idmap_iterator(groups->map);

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

				SHARED_LOCK(&groups->mutex, {

					Group* group = idmap_get(groups->map, gid);

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
