
/*
 *  Copyright (C) 2024 magistermaks
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

// the port on which URP operates
#define URP_VERSION  1
#define URP_REVISION 0

// user-to-relay packets
#define U2R_MAKE             0x00 // create user group
#define U2R_JOIN             0x01 // join given user group
#define U2R_QUIT             0x02 // leave user group
#define U2R_BROD             0x03 // broadcast message too all but the specified user in a your group
#define U2R_SEND             0x04 // send message to the specified user in a your group
#define U2R_GETS             0x05 // get setting value of the given key
#define U2R_SETS             0x06 // set setting value of the given key
#define U2R_KICK             0x07 // kick user from the group, can be send only by the host

// relay-to-user packets
#define R2U_WELC             0x10 // the first packet send to newly connected users
#define R2U_TEXT             0x11 // delivers a message to the recipiant
#define R2U_MADE             0x12 // a response to the U2R_MAKE/U2R_JOIN packets
#define R2U_JOIN             0x13 // send to the host when a new users joins the group
#define R2U_LEFT             0x14 // send to the host when a user leaves the group
#define R2U_STAT             0x15 // send to the user when their role changes or when they issue a command while in the wrong role
#define R2U_VALS             0x16 // a response to the U2R_GETS/U2R_SETS packets
//#define R2U_DROP           0x17 // gives a reason for a server disconnect, send just before closing the socket

// Special IDs
#define NULL_GROUP 0
#define NULL_USER 0

// Setting keys
#define SETK_INVALID         0x00 // returned with the value of the key, when an invalid key is send
#define SETK_GROUP_PASS      0x01 // group password
#define SETK_GROUP_FLAGS     0x02 // group flags, see FLAG_GROUP_* defines below
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
#define FLAG_GROUP_LOCK      0x01 // enabling this group flag blocks the ability of new members to join
#define FLAG_GROUP_NOSEND    0x02 // blocks the ability of all but the host to issue the U2R_SEND command
#define FLAG_GROUP_NOBROD    0x04 // blocks the ability of all but the host to issue the U2R_BROD command
#define FLAG_GROUP_NOP2P     0x08 // when enabled the only valid target for U2R_SEND becomes the host
#define FLAG_GROUP_BINARY    0x10 // indicates for the client that the group uses binary communication
#define FLAG_GROUP_ENCRYPTED 0x20 // indicates for the client that the group uses encryption of some sort
