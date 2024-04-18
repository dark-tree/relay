
#pragma once

// C Standard Library
#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// POSIX
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>

#define streq(a, b) (strcmp(a, b) == 0)
