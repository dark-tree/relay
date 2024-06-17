cmake_minimum_required(VERSION 3.9)
project(URP LANGUAGES C)

set(CMAKE_C_STANDARD 11)

include_directories(src .)

set(CLIENT_SOURCES
	src/client/client.c
)

set(SERVER_SOURCES
	src/server/group.c
	src/server/mutex.c
	src/server/sequence.c
	src/server/server.c
	src/server/user.c
	src/server/tcps.c
	src/server/store.c
	src/server/config.c
)

set(COMMON_SOURCES
	src/common/logger.c
	src/common/map.c
	src/common/set.c
	src/common/vec.c
	src/common/stream.c
	src/common/input.c
	src/common/util.c
)

add_library(common STATIC ${COMMON_SOURCES})

add_executable(server ${SERVER_SOURCES})
target_link_libraries(server PRIVATE common)

add_executable(client ${CLIENT_SOURCES})
target_link_libraries(client PRIVATE common)
