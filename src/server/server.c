
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
#include <server/config.h>
#include <server/servers.h>

IdStore* users;
IdStore* groups;

void* user_thread(void* context) {

	User * const user = (User*) context;
	NioStream* stream = &user->stream;

	// FIXME
	uint8_t brand[64] = "My Little Relay";

	log_info("User #%d connected over %s\n", user->uid, stream->impl->id);

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
	util_mstime(&default_read, 200);

	while (nio_open(stream)) {

		uint8_t id;

		nio_flush(stream);
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

			// a race condition in access to the write socket
			// here is extremally unlikely, but not impossible
			// for example if a new user joins before the R2U_MAKE
			// packet is send to host
			WRITE_LOCK(user, {
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
			const uint32_t password = nio_read32(stream);

			log_debug("Received from user #%d: U2R_JOIN {gid=%u, pass=%u}\n", user->uid, gid, password);

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

					WRITE_LOCK(user, {
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

							if (group->password != password) {
								WRITE_LOCK(user, {
									nio_write8(stream, R2U_MADE);
									nio_write8(stream, FROM_JOIN | STAT_ERROR_PASSWORD);
									nio_write32(stream, NULL_GROUP);
								});

								goto inner_fail;
							}

							if (group->flags & FLAG_GROUP_LOCK) {
								WRITE_LOCK(user, {
									nio_write8(stream, R2U_MADE);
									nio_write8(stream, FROM_JOIN | STAT_ERROR_LOCK);
									nio_write32(stream, NULL_GROUP);
								});

								goto inner_fail;
							}

							// this check of member count vs. member limit looks
							// unsafe (we first check member count and only after that
							// (non atomically) increment member count) but in actuallity
							// this whole block is protected by the group's master_mutex
							// so it is synchronized and safe as-is
							if (group->refcnt >= group->member_limit) {
								WRITE_LOCK(user, {
									nio_write8(stream, R2U_MADE);
									nio_write8(stream, FROM_JOIN | STAT_ERROR_FULL);
									nio_write32(stream, NULL_GROUP);
								});

								goto inner_fail;
							}

							// add new member
							idvec_put(&group->members, user);

							group->refcnt ++;
							user->role = ROLE_MEMBER;
							user->group = group;

							// notify group host
							WRITE_LOCK(group->host, {
								nio_write8(&group->host->stream, R2U_JOIN);
								nio_write32(&group->host->stream, user->uid);
							});

							// notify new member
							WRITE_LOCK(user, {
								nio_write8(stream, R2U_MADE);
								nio_write8(stream, FROM_JOIN | STAT_OK);
								nio_write32(stream, group->gid);

								nio_write8(stream, R2U_STAT);
								nio_write8(stream, ROLE_MEMBER);
							});

							log_info("User #%d joined group #%d\n", user->uid, group->gid);

						} else {

							WRITE_LOCK(user, {
								nio_write8(stream, R2U_MADE);
								nio_write8(stream, status_closed);
								nio_write32(stream, NULL_GROUP);
							});

						}

						inner_fail:

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

			Group* group = user->group;
			uint32_t host = group->uid;
			bool length = (len > group->payload_limit);

			// host can do everything so we only check for members
			if ((host != user->uid) || length) {

				uint32_t flags = group->flags;

				// U2R_SEND packets are blocked OR user-to-user U2R_SEND packets are blocked
				if (length || (flags & FLAG_GROUP_NOSEND) || ((flags & FLAG_GROUP_NOP2P) && (uid != host))) {

					// as before we need to read the message anyway
					nio_skip(stream, len);
					continue;
				}

			}

			// TODO opti: potentially optimize user access
			SHARED_LOCK(&users->mutex, {

				User* target = idmap_get(users->map, uid);

				if (target && target->group == group) {

					NioBlock block = nio_block(stream, len);
					nio_readbuf(stream, &block);

					WRITE_LOCK(target, {

						NIO_CORK(&target->stream, {

							// send header once
							nio_write8(&target->stream, R2U_TEXT);
							nio_write32(&target->stream, user->uid);
							nio_write32(&target->stream, len);

							nio_writebuf(&target->stream, &block);

							while (block.remaining) {

								// TODO spec: add guard byte at every section (allowing the packet to be ended early)
								// TODO opti: packet interweaving

								nio_readbuf(stream, &block);
								nio_writebuf(&target->stream, &block);

							}

						});

					});

					nio_flush(&target->stream);

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

			if (user_verify(user, ROLE_HOST | ROLE_MEMBER)) {

				// packet was send while user was in invalid state
				// we have to read the message bytes from the stream
				// to stay synchronized
				nio_skip(stream, len);

				continue;
			}

			Group* group = user->group;
			bool length = (len > group->payload_limit);

			// host can do everything so we only check for members
			if (length || (group->uid != user->uid) && (group->flags & FLAG_GROUP_NOBROD)) {

				// as before we need to read the message anyway
				nio_skip(stream, len);

				continue;
			}

			// wait for master group lock as
			// we will lock more than one user lock
			// and need the group to be constant
			SEMAPHORE_LOCK(&group->master_mutex, {

				NioBlock block = nio_block(stream, len);
				nio_readbuf(stream, &block);

				// lock targets
				IDVEC_FOREACH(User*, target, group->members) {
					if (target->uid != uid) {
						sem_wait(&NIO_STATE(&target->stream)->mutex);
						nio_cork(&target->stream, true);

						// send header once
						nio_write8(&target->stream, R2U_TEXT);
						nio_write32(&target->stream, user->uid);
						nio_write32(&target->stream, len);
					}
				}

				IDVEC_FOREACH(User*, initial, group->members) {
					if (initial->uid != uid) {
						nio_writebuf(&initial->stream, &block);
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
						sem_post(&NIO_STATE(&target->stream)->mutex);
						nio_flush(&unlock->stream);
					}
				}

			});

		}

		if (id == U2R_SETS) {

			uint32_t key = nio_read32(stream);
			uint32_t val = nio_read32(stream);

			log_debug("Received from user #%d: U2R_SETS {key=%u, val=%u}\n", user->uid, key, val);

			uint32_t* setting = user_setting_get(user, key, true);

			if (setting) {
				*setting = val;
				user_setting_send(user, key, *setting);
			}

			continue;
		}

		if (id == U2R_GETS) {

			uint32_t key = nio_read32(stream);

			log_debug("Received from user #%d: U2R_GETS {key=%u}\n", user->uid, key);

			uint32_t* setting = user_setting_get(user, key, false);

			if (setting) {
				user_setting_send(user, key, *setting);
			}

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

int main() {

	OpenSSL_add_all_algorithms();

	Config cfg;

	config_default(&cfg);
	config_load(&cfg, "server.cfg");

	// set logger level
	log_setlv(cfg.level);

	// we need to block it so that write() doesn't trigger signals on error
	// as that would make it harder for use to detect errors
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		log_fatal("Failed to set a ignore handler for SIGPIPE!\n");
		exit(-1);
	}

	users = store_create(cfg.uids);
	groups = store_create(cfg.gids);

	ServerPool servers;
	servers.user_thread = user_thread;

	server_start(&servers, &cfg);

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
			server_stop(&servers);
		}

		if (streq(buffer, "users")) {

			if (users->counter == 0) {
				printf("There are currently no connected users.\n");
				continue;
			}

			printf("List of all %d users:\n", users->counter);

			// we lock the groups mutex as well here
			// so that we can guarantee that once we copy a group pointer from a user it
			// won't become invalid
			SHARED_LOCK(&groups->mutex, {
				SHARED_LOCK(&users->mutex, {

					IdMapIterator iter = idmap_iterator(users->map);

					while (idmap_has(&iter)) {
						User* user = (User*) idmap_next(&iter);
						Group* group = user->group;

						if (user->role == ROLE_CONNECTED || (group == NULL)) {
							printf(" * user #%d connected\n", user->uid);
							continue;
						}

						if (user->role == ROLE_MEMBER) {
							printf(" * user #%d member of group #%d\n", user->uid, group->gid);
							continue;
						}

						if (user->role == ROLE_HOST) {
							printf(" * user #%d host of group #%d\n", user->uid, group->gid);
							continue;
						}
					}

				});
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

						SEMAPHORE_LOCK(&group->master_mutex, {

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
