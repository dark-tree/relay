
#include "mutex.h"

void mutex_init(SharedMutex* mutex) {
	sem_init(&mutex->shared, 0, 1);
	sem_init(&mutex->unique, 0, 1);
	sem_init(&mutex->common, 0, 1);
	mutex->readers = 0;
}

void mutex_close(SharedMutex* mutex) {
	sem_destroy(&mutex->shared);
	sem_destroy(&mutex->unique);
	sem_destroy(&mutex->common);
}

void mutex_shared_lock(SharedMutex* mutex) {
	sem_wait(&mutex->common);
	sem_wait(&mutex->shared);

	mutex->readers ++;

	if (mutex->readers == 1) {
		sem_wait(&mutex->unique);
	}

	sem_post(&mutex->shared);
	sem_post(&mutex->common);
}

void mutex_shared_unlock(SharedMutex* mutex) {
	sem_wait(&mutex->shared);

	mutex->readers --;

	if (mutex->readers == 0) {
		sem_post(&mutex->unique);
	}

	sem_post(&mutex->shared);
}

void mutex_unique_lock(SharedMutex* mutex) {
	sem_wait(&mutex->common);
	sem_wait(&mutex->unique);
}

void mutex_unique_unlock(SharedMutex* mutex) {
	sem_post(&mutex->unique);
	sem_post(&mutex->common);
}
