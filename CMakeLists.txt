include_directories(lib/asio-1.22.1/include src/ src/common)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -pthread")

add_executable(server src/server/server.cpp src/server/users.cpp src/server/packets.cpp src/common/writer.cpp src/common/parser.cpp)
add_executable(client src/client/client.cpp src/client/packets.cpp src/common/writer.cpp src/common/parser.cpp)

