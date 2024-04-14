
#pragma once
#include "external.h"

#include <common/map.h>
#include <server/mutex.h>
#include <server/sequence.h>

struct User_tag;
struct Group_tag;

typedef struct IdStore_tag {

	IdMap* map;
	SharedMutex mutex;
	IdSequence sequence;
	atomic_int counter;

} IdStore;

/// Create a new synchronized storage object
/// it needs to be later freed with store_free
IdStore* store_create(IdSeqMode mode);

///
///
void store_free(IdStore* store);

/// Puts a new instance of a user into the synchronized storage
/// refere to the user_create function
struct User_tag* store_putuser(IdStore* store, int connfd);

/// Puts a new instance of a group into the synchronized storage
/// refere to the group_create function
struct Group_tag* store_putgroup(IdStore* store, struct User_tag* user);

/// Removes a user or group object of the given ID
/// from the synchronized storage
void store_remove(IdStore* store, uint32_t id);
