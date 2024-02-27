
#pragma once

#include <cstdint>
#include <iostream>
#include <thread>
#include <utility>
#include <cassert>
#include <random>
#include <shared_mutex>
#include <chrono>
#include "asio.hpp"

#include "logger.hpp"

using asio::ip::tcp;

// version of the User Relay Protocol used by this server
#define URP_VERSION 1

#define U2R_MAKE 0x00 // create new user group
#define U2R_JOIN 0x01 // join a user group
#define U2R_QUIT 0x02 // exit a user group
#define U2R_BROD 0x03 // broadcast a message in a user group
#define U2R_SEND 0x04 // send a message to a specific user of a user group
#define U2R_GETS 0x05 // data read
#define U2R_SETS 0x06 // data write

#define R2U_WELC 0x10 // send to newly joined users
#define R2U_TEXT 0x11 // message received
#define R2U_MADE 0x12 // new user group made
#define R2U_JOIN 0x13 // user joined group
#define R2U_LEFT 0x14 // user left group (to host)
#define R2U_EXIT 0x15 // user left group (to user)
#define R2U_VALS 0x16 // data value

#define LEVEL_NO_ONE 0
#define LEVEL_MEMBER 1
#define LEVEL_HOST 2
#define NULL_GROUP 0
#define NULL_USER 0

#define MADE_STATUS_MADE 0b000'1 // the group was created
#define MADE_STATUS_JOIN 0b001'1 // the group was joined
#define MADE_STATUS_PASS 0b010'0 // invalid authentication 
#define MADE_STATUS_LOCK 0b011'0 // joining is disabled
#define MADE_STATUS_FAIL 0b100'0 // error occured

#define DATA_KEY_PASS 1 // group password
#define DATA_KEY_FLAG 2 // group flags

#define GROUP_FLAG_LOCK 0b0001 // if set then he group can't be joined even with correct password
#define GROUP_FLAG_LIST 0b0010 // if set then the group will apear in the public group list
#define GROUP_FLAG_HOST 0b0100 // if set then only the host can send messages
#define GROUP_FLAG_BINC 0b1000 // if set marks the group for binary communication

// helper macro
#define scase(val, ...) case val: { __VA_ARGS__ } break;

