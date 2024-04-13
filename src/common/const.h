
#pragma once

// the port on which URP operates
#define URP_PORT 9686
#define URP_VERSION 1
#define URP_REVISION 0

// user-to-relay packets
#define U2R_MAKE 0x00 // create new user group
#define U2R_JOIN 0x01 // join a user group
#define U2R_QUIT 0x02 // exit a user group
#define U2R_BROD 0x03 // broadcast a message in a user group
#define U2R_SEND 0x04 // send a message to a specific user of a user group
#define U2R_GETS 0x05 // data read
#define U2R_SETS 0x06 // data write
#define U2R_KICK 0x07 //

// relay-to-user packets
#define R2U_WELC 0x10 // send to newly joined users
#define R2U_TEXT 0x11 // message received
#define R2U_MADE 0x12 // new user group made
#define R2U_JOIN 0x13 // user joined group
#define R2U_LEFT 0x14 // user left group (to host)
#define R2U_STAT 0x15 //
#define R2U_VALS 0x16 //
//#define R2U_DROP    // gives a reason for a server disconnect, send just before closing the socket

// Special IDs
#define NULL_GROUP 0
#define NULL_USER 0

// Setting keys
#define SETK_INVALID         0x00
#define SETK_GROUP_PASS      0x01 //
#define SETK_GROUP_FLAGS     0x02 //
#define SETK_GROUP_MEMBERS   0x03 // maximum number of group members
#define SETK_GROUP_PAYLOAD   0x04 // maximum size of payload in U2R_SEND or U2R_BROD

// Status in the R2U_MADE
#define STAT_OK              0x00 // operation completed succesfully
#define STAT_ERROR_INVALID   0x01 // no such group or the group is closing
#define STAT_ERROR_PASSWORD  0x02 // invalid group password
#define STAT_ERROR_FULL      0x03 // the group member count reached the group member limit
#define STAT_ERROR_LOCK      0x04 // the group is locked and can't be joined
#define STAT_ERROR_SATURATED 0x05 // maximum number of concurent groups reached

// R2U_MADE origin
#define FROM_MAKE            0x00 //
#define FROM_JOIN            0x10 //

// Group flags
#define FLAG_GROUP_LOCK      0b01
#define FLAG_GROUP_NOSEND    0b02
#define FLAG_GROUP_NOBROD    0b04
#define FLAG_GROUP_NOP2P     0b08
#define FLAG_GROUP_BINARY    0b10
