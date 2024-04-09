
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
#define U2R_KICK 0x07

// relay-to-user packets
#define R2U_WELC 0x10 // send to newly joined users
#define R2U_TEXT 0x11 // message received
#define R2U_MADE 0x12 // new user group made
#define R2U_JOIN 0x13 // user joined group
#define R2U_LEFT 0x14 // user left group (to host)
#define R2U_STAT 0x15 //
#define R2U_VALS 0x16 // data value

// special IDs
#define NULL_GROUP 0
#define NULL_USER 0

// Status in the R2U_MADE
#define STAT_OK              0x0 // operation completed succesfully 
#define STAT_ERROR_INVALID   0x1 // no such group or the group is closing
#define STAT_ERROR_PASSWORD  0x2 // invalid group password
#define STAT_ERROR_FULL      0x3 // the group member count reached the group memebr limit
#define STAT_ERROR_LOCK      0x4 // the group is locked and can't be joined 
#define STAT_ERROR_SATURATED 0x5 // maximum number of concurent groups reached
#define FROM_MAKE            0x0 
#define FROM_JOIN            0x1 

#define MADE_COMPOSE(from, stat) ((from << 4) | stat)

