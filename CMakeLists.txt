cmake_minimum_required(VERSION 3.9)
project(URP LANGUAGES C)

find_package(OpenSSL REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
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
	src/server/servers.c
)

set(COMMON_SOURCES
	src/common/logger.c
	src/common/map.c
	src/common/set.c
	src/common/vec.c
	src/common/stream.c
	src/common/input.c
	src/common/util.c
	src/common/http.c
	src/common/network/tcp.c
	src/common/network/ws.c
)

add_library(common STATIC ${COMMON_SOURCES})

add_executable(server ${SERVER_SOURCES})
target_link_libraries(server PRIVATE common OpenSSL::Crypto OpenSSL::SSL)

add_executable(client ${CLIENT_SOURCES})
target_link_libraries(client PRIVATE common OpenSSL::Crypto OpenSSL::SSL)
