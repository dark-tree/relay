
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

/// Frees memory alloced for the synchronized storage and any internally
/// used structures. The object is no longer usable after this call.
void store_free(IdStore* store);

/// Creates a new instance of a user and emplaces it into the
/// synchronized storage. Refere to the user_create function
struct User_tag* store_putuser(IdStore* store, int connfd);

/// Creates a new instance of a group and emplaces it into the
/// synchronized storage. Refere to the group_create function
struct Group_tag* store_putgroup(IdStore* store, struct User_tag* user);

/// Removes a user or group object of the given ID
/// from the synchronized storage
void store_remove(IdStore* store, uint32_t id);
