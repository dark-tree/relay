
#include "group.h"
#include "user.h"

Group* group_create(uint32_t gid, User* user) {
	Group* group = malloc(sizeof(Group));

	group->gid = gid;
	group->uid = user->uid;
	group->host = user;
	group->password = 0;
	group->flags = 0;

	// reference count is used for group disbanding
	group->refcnt = 1;
	group->close = false;

	// host is also considered a member
	group->members = idvec_create(16);
	idvec_put(&group->members, user);

	// deletion_mutex shall be initially locked
	sem_init(&group->master_mutex, 0, 1);
	sem_init(&group->disband_mutex, 0, 0);

	return group;
}

void group_free(Group* this) {

	idvec_free(&this->members);
	sem_destroy(&this->master_mutex);
	sem_destroy(&this->disband_mutex);

	// reset internal state
	this->host = NULL;
	this->gid = 0;
	this->uid = 0;

	free(this);

}

int group_find(Group* group, uint32_t uid) {
	IDVEC_FOREACH(User*, user, group->members) {
		if (user->uid == uid) return i;
	}

	return -1;
}

void* group_cleanup(void* this) {

	Group* group = this;

	uint32_t gid = group->gid;
	uint32_t uid = group->uid;

	// wait for the last user to leave,
	// this mutex is initially locked and will block here until
	// the last user to leave the group unlockes it
	sem_wait(&group->disband_mutex);

	// remove the group from the idmap first so that no pointer can
	// point at deallocated memory. This lock is also important to the handling of
	// the U2R_JOIN packet, and guarantees that the group will not be removed during usage.
	UNIQUE_LOCK(&group_mutex, {
		free(idmap_remove(groups, group->gid));
	});

	// now safely free the group
	// TODO should we put group_free into group_mutex UNIQUE_LOCK
	group_free(group);

	log_info("User #%d disbanded group #%d\n", uid, gid);
	return NULL;

}

void group_exit(Group* group, User* user) {

	// notify self
	SEMAPHORE_LOCK(&user->write_mutex, {

		user->role = ROLE_CONNECTED;
		user->group = NULL;

		nio_write8(&user->stream, R2U_STAT);
		nio_write8(&user->stream, ROLE_CONNECTED);

	});

	// we need to remove ourselves from the memeber set
	// so that at no point there can be an invalid pointer in it
	SEMAPHORE_LOCK(&group->master_mutex, {

		if (group->uid == user->uid) {

			// host in no longer a member so clear the host pointer
			// we internally leave group->uid intact here as it's safe to dereference
			// using the hashmap and allows us to use non-blocking logic in handling of some packets
			group->host = NULL;

		}

		const int index = group_find(group, user->uid);

		// this check should always pass
		// but lets make it anyway, just to be sure
		if (index != -1) {
			idvec_remove(&group->members, index);
		}

	});

	// notify host but only if the group isn't closing
	// (this method is also called when user is kicked)
	// otherwise there is no point (and way, as the host pointer
	// can be gone already)
	if (group->host && !group->close) {

		User* host = group->host;

		SEMAPHORE_LOCK(&group->host->write_mutex, {

			nio_write8(&host->stream, R2U_LEFT);
			nio_write32(&host->stream, user->uid);

		});

	}

	// decrements an atomic reference counter
	// when it reaches 0 (meaning all users left), we
	// unlock the cleanup thread. Note that the sem_post will only run once
	// as the atomicity of the counter guarantiees that the 'if' will not
	// execute for more than one thread.
	if ((-- group->refcnt) == 0) {
		sem_post(&group->disband_mutex);
	}

}

void group_disband(Group* group) {

	// host is safe to access here as we are the only
	// ones that can change it
	User* user = group->host;

	// start group closing, upon noticing the close
	// flag users will start removing themselves from the group
	group->close = true;

	// remove host from the group
	group_exit(group, user);
	log_info("User #%d left group #%d as it is being disbanded\n", user->uid, group->gid);

	pthread_t thread;
	if (pthread_create(&thread, NULL, group_cleanup, group)) {

		log_error("Failed to start group cleanup thread, falling back to a synchronous method!\n");
		group_cleanup(group);

	}

}

bool group_remove(Group* group, uint32_t uid) {

	bool found = false;

	SEMAPHORE_LOCK(&group->master_mutex, {

		const int index = group_find(group, uid);

		// this check can fail as the UID
		// given to this method is not sanitized.
		if (index != -1) {

			User* user = group->members.data[index];

			user->exit = true;
			found = true;

		}

	});

	return found;

}
