
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

IdStore* store_create(IdSeqMode mode);

void store_free(IdStore* store);

struct User_tag* store_putuser(IdStore* store, int connfd);

struct Group_tag* store_putgroup(IdStore* store, struct User_tag* user);

void store_remove(IdStore* store, uint32_t id);
