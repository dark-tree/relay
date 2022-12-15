# User Relay Protocol
Version 1

URP is a communication protocol working on top of TCP/IP that allows for an emulation of a pear-to-pear like network. Users connected to a User Relay can group themselves into "User Groups" that can then exchange binary messages with a size of up to 65 kB (65531 bytes).

### Packet Structure
Every command send to or from a relay starts with a 3 byte long header, consisting of two fields
- type, 1 unsigned 8 bit integer (1 byte) representing the packet type
- size, 1 unsigned 16 bit integer (2 bytes) representing the size of the packet body in bytes

And a packet body whose length is defined by the head, the exact meaning of the data in this section is packet type dependent.

### Packet Types
Depending on the packet type defined in the header the packet and its body should be interpreted according to this table:

| Name | Type | Structure | Description |
| - | - | - | - |
| `U2R_MAKE` | 0x00 | N/A | Create new user group |
| `U2R_JOIN` | 0x01 | uint32: gid | Join a user group of given ID |
| `U2R_QUIT` | 0x02 | N/A | Leave the current user group |
| `U2R_BROD` | 0x03 | uint32: uid, bytes: msg | Broadcast a message to all members of a group, except for the one with the given ID |
| `U2R_SEND` | 0x04 | uint32: uid, bytes: msg | Send a message to a user with given ID |
| `R2U_WELC` | 0x10 | uint32: uid, uint32: ver | Send to newly connected users |
| `R2U_TEXT` | 0x11 | bytes: msg | Transmits the incoming message |
| `R2U_MADE` | 0x12 | uint32: gid | Notifies the host of group creation |
| `R2U_JOIN` | 0x13 | uint32: uid | Notifies the host that a user has joined the group |
| `R2U_LEFT` | 0x14 | uint32: uid | Notifies the host that a user has left the group |

**Notes:**
- Packets starting with `U2R` are send by the user to relay, and packets starting with `R2U` are send by relay to the user.
- The two fields of the `R2U_WELC` packet are guarantied to stay the same for all future URP versions.
- "N/A" in the "Structure" column indicates that this packet has no arguments (size should be equal to 0).
- The `uid` parameter of the `U2R_BROD` command can be set to `0` to send to all users

### Users
A relay user can be in one of 3 states:

| Name | Code | Description | Allowed Commands |
| - | - | - | - |
| `NO_ONE` | 0 | When not in any group | `U2R_MAKE` `U2R_JOIN` |
| `MEMBER` | 1 | When in a group | `U2R_QUIT` `U2R_BROD` `U2R_SEND` |
| `HOST` | 2 | When hosting a group | `U2R_QUIT` `U2R_BROD` `U2R_SEND` |

### Groups
A user group is a collection of users created using the `U2R_MAKE` packet by the group host,
users can join and leave the group at any time. If the host leaves the group it is disbanded (all other members are removed and the group is deleted). Host is also considered a member of his group. When users are in a group thay can communicated with the `U2R_BROD` and `U2R_SEND` packets to other users in the same group.
