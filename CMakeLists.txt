include_directories(lib/asio-1.22.1/include)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -pthread")

add_executable(main src/main.cpp src/users.cpp src/packets.cpp src/writer.cpp)

