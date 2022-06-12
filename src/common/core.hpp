
#pragma once

#include <cstdint>
#include <iostream>
#include <thread>
#include <utility>
#include <cassert>
#include <shared_mutex>
#include "asio.hpp"

using asio::ip::tcp;

// version of the User Relay Protocol used by this server
#define URP_VERSION 1

#define U2R_MAKE 0x00 // create new user group
#define U2R_JOIN 0x01 // join a user group
#define U2R_QUIT 0x02 // exit a user group
#define U2R_BROD 0x03 // brodcast a message in a user group
#define U2R_SEND 0x04 // send a message to a specific user of a user group

#define R2U_WELC 0x10 // send to newly joined users
#define R2U_TEXT 0x11 // message recived
#define R2U_MADE 0x12 // new user group made
#define R2U_JOIN 0x13 // user joind group
#define R2U_LEFT 0x14 // user left group

// helper macro
#define scase(val, ...) case val: { __VA_ARGS__ } break;

