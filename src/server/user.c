
#include "user.h"

User* user_create(uint32_t uid, int connfd) {
	User* user = malloc(sizeof(User));

	user->uid = uid;
	user->role = ROLE_CONNECTED;
	user->group = NULL;
	user->exit = false;

	nio_create(&user->stream, connfd, 0x1000);
	sem_init(&user->write_mutex, 0, 1);

	return user;
}

void user_free(User* user) {
	nio_free(&user->stream);
	sem_destroy(&user->write_mutex);
	free(user);
}
