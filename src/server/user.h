
#pragma once
#include "external.h"

#include <common/stream.h>

typedef enum {
	ROLE_CONNECTED = 1,
	ROLE_MEMBER    = 2,
	ROLE_HOST      = 4
} UserRole;

typedef struct User_tag {
	uint32_t uid;
	UserRole role;
	struct Group_tag* group;
	NioStream stream;

	volatile bool exit;

	// This mutex is used to guard agains two threads
	// writing at the same time to the same connection.
	// To protect against a deadlock during locking of those
	// mutexes the Master Group Lock is used, learn more in group.h
	sem_t write_mutex;
} User;

/// Allocates a new user and returns a pointer to it, the user will be
/// assigned the passed in UID and connection handle.
User* user_create(uint32_t uid, int connfd);

/// Frees memory alloced for the user and any internally
/// used structures. The object is no longer usable after this call.
void user_free(User* user);

int user_verify(User* user, uint8_t role);

void user_kick(User* user, uint32_t uid);

void user_quit(User* user);

uint32_t* user_setting(User* user, uint32_t key, bool write);
