//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Shm.h
//
// Purpose-
//       Data areas associated with Shm.cpp
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef SCHEDULE_H_INCLUDED
#define SCHEDULE_H_INCLUDED

#include <sys/types.h>

//----------------------------------------------------------------------------
// Typedefs
//----------------------------------------------------------------------------
typedef unsigned       Offset;      // Offset (from SharedStorageRegion origin)

//----------------------------------------------------------------------------
//
// Struct-
//       SharedStorageRegion
//
// Purpose-
//       Define the shared storage region.
//
//----------------------------------------------------------------------------
struct SharedStorageRegion {        // SharedStorageRegion
//----------------------------------------------------------------------------
// SharedStorageRegion::Typedefs and enumerations
//----------------------------------------------------------------------------
enum                                // Constants for parameterization
{
   VERSIONID=              20070101,// Version identifier
   MAX_SIZE=                32*4096 // The default size of the region (16M)
}; // enum

enum FSM                            // Finite State Machine
{
   FSM_RESET= 0,                    // Reset, not initialized
   FSM_BOOT,                        // Started by -boot, active
   FSM_INIT,                        // Started by -init, active
   FSM_TERM,                        // Terminated by -term, active
   FSM_WAIT,                        // Terminated by -wait, active
   FSM_TERMINATED                   // Terminated
}; // enum FSM

//----------------------------------------------------------------------------
// SharedStorageRegion::Attributes
//----------------------------------------------------------------------------
   char                ident[8];    // Control block identifier
   unsigned            versionid;   // Version identifier
   unsigned            tokenid;     // FTOK token identifier
   pid_t               waitforPid;  // WaitforThread sub-process identifier
   unsigned            fsm;         // Current state
   unsigned            size;        // Total size of area, in bytes

   Offset              freeList;    // The list of free Command blocks
   Offset              activeList;  // The list of active Commands
   Offset              unseenList;  // The list of unseen Commands

   char                pool[MAX_SIZE]; // Free storage pool
}; // class SharedStorageRegion

//----------------------------------------------------------------------------
//
// Struct-
//       Command
//
// Purpose-
//       Define a command.
//
//----------------------------------------------------------------------------
struct Command {                    // Command
//----------------------------------------------------------------------------
// Command::Typedefs and enumerations
//----------------------------------------------------------------------------
enum                                // Constants for parameterization
{
   CMD_SIZE=                   4096 // The maximum number of bytes in a command
}; // enum

enum FSM                            // Finite State Machine
{
   FSM_RESET= 0,                    // Reset, not examined
   FSM_WAITING,                     // Waiting for dependent command(s)
   FSM_ACTIVE,                      // Active, running
   FSM_COMPLETE                     // Complete
}; // enum FSM

//----------------------------------------------------------------------------
// Command::Attributes
//----------------------------------------------------------------------------
   Offset              next;        // Chain pointer
   pid_t               pid;         // Process identifier
   unsigned            fsm;         // State
   unsigned            compCode;    // Completion code (when fsm==FSM_COMPLETE)
   Offset              _0001[1];    // Reserved for alignment

   Offset              name;        // The name of this command
   Offset              code;        // The command
   Offset              deps;        // The dependency list

   char                command[CMD_SIZE]; // The command descriptor
}; // struct Command

#endif // SCHEDULE_H_INCLUDED
