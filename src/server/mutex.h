
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

#pragma once
#include "external.h"

typedef struct {
	sem_t shared;
	sem_t unique;
	sem_t common;

	int readers;
} SharedMutex;

/// Initialize the shared mutex,
/// all the locks are initially open.
void mutex_init(SharedMutex* mutex);

/// Delete the shared mutex and free,
/// any internally used structures.
void mutex_close(SharedMutex* mutex);

/// Acquire a read-only access to the gated resource,
/// multiple threads are allowed to pass this lock.
void mutex_shared_lock(SharedMutex* mutex);

/// Release a read-only access to the gated resource,
/// for better readability consider using the SHARED_LOCK macro.
void mutex_shared_unlock(SharedMutex* mutex);

/// Acquire a read-write access to the gated resource,
/// only one thread can pass this lock, entering it closes all other shared locks.
void mutex_unique_lock(SharedMutex* mutex);

/// Release a read-write access to the gated resource,
/// for better readability consider using the UNIQUE_LOCK macro.
void mutex_unique_unlock(SharedMutex* mutex);

/// shared lock macro helper
#define SHARED_LOCK(mutex, ...) \
	mutex_shared_lock(mutex); \
	{ \
		__VA_ARGS__ \
	} \
	mutex_shared_unlock(mutex);

/// unique lock macro helper
#define UNIQUE_LOCK(mutex, ...) \
	mutex_unique_lock(mutex); \
	{ \
		__VA_ARGS__ \
	} \
	mutex_unique_unlock(mutex);

/// semaphore lock macro helper
#define SEMAPHORE_LOCK(mutex, ...) \
	sem_wait(mutex); \
	{ \
		__VA_ARGS__ \
	} \
	sem_post(mutex);
