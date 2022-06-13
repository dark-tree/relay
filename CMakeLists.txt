include_directories(lib/asio-1.22.1/include src/ src/common)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -pthread")

set(SERVER_SOURCE src/server/server.cpp src/server/packets.cpp src/server/users.cpp)
set(CLIENT_SOURCE src/client/client.cpp src/client/packets.cpp )
set(COMMON_SOURCE src/common/writer.cpp src/common/parser.cpp src/common/logger.cpp)

add_executable(server ${SERVER_SOURCE} ${COMMON_SOURCE})
add_executable(client ${CLIENT_SOURCE} ${COMMON_SOURCE})

