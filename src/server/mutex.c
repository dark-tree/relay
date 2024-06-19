
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

#include "mutex.h"

void mutex_init(SharedMutex* mutex) {
	sem_init(&mutex->shared, 0, 1); // TODO: replace with atomic int
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
