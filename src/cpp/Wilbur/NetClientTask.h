//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       NetClientTask.h
//
// Purpose-
//       The host-specific NetClientItem processor.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       TODO: Needs work, and lots of it.
//
//----------------------------------------------------------------------------
#ifndef NETCLIENTTASK_H_INCLUDED
#define NETCLIENTTASK_H_INCLUDED

#ifndef NETCLIENT_H_INCLUDED
#include "NetClient.h"              // Includes com/Dispatch.h (string, Barrier)
#endif

#include <com/Clock.h>              // For KeepAlive clock
#include "HttpCached.h"
#include "Robots.h"

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class NetClient;                    // Owner class
class NetClientItem;                // Item class

//----------------------------------------------------------------------------
//
// Class-
//       NetClientTask
//
// Purpose-
//       Host-specific NetClientItem processor.
//
//----------------------------------------------------------------------------
class NetClientTask : public DispatchTask { // Host-specific NetClientItem processor
friend class NetClient;

//----------------------------------------------------------------------------
// NetClientTask::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum FSM                            // Finite State Machine States
{  FSM_RESET                        // RESET, no operation pending
,  FSM_READY                        // READY, operation in progress
,  FSM_TIMER                        // TIMER, KeepAlive timer chase
,  FSM_CLOSE                        // CLOSE, terminating
}; // enum FSM

//----------------------------------------------------------------------------
// NetClientTask::Attributes
//----------------------------------------------------------------------------
public:
NetClient*             owner;       // Our NetClient
std::string            client;      // The associated client

Barrier                barrier;     // Protects data areas
int                    fsm;         // Finite State Machine
HttpCached             cached;      // Working cached HttpSource
Robots                 robots;      // Robots.txt control

// KeepAlive controls
Clock                  alive;       // KeepAlive last virtual read time
Clock                  clock;       // KeepAlive last network read time
void*                  token;       // KeepAlive delay cancel token
NetClientItem*         kaItem;      // KeepAlive timer work Item
DispatchWait*          kaWait;      // KeepAlive timer Wait object

//----------------------------------------------------------------------------
// NetClientTask::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~NetClientTask( void );          // Destructor
   NetClientTask(                   // Contructor
     NetClient*        owner,       // Owner NetClient
     NetClientItem*    item);       // First NetClientItem

//----------------------------------------------------------------------------
// NetClientTask::Accessors
//----------------------------------------------------------------------------
public:
static std::string                  // Resultant
   getClient(                       // Get associated client
     NetClientItem*    item);       // For this NetClientItem

//----------------------------------------------------------------------------
// NetClientTask::Methods
//----------------------------------------------------------------------------
public:
void
   setFSM(                          // Change the FSM
     int               state);      // To this state

void
   timer( void );                   // Handle KeepAlive timeout

virtual void
   work(                            // Operate on a work Item
     DispatchItem*     item);       // The work Item
}; // class NetClientTask

#endif // NETCLIENTTASK_H_INCLUDED
