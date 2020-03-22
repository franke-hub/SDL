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
//       NetClient.h
//
// Purpose-
//       Process NetClientItem requests by routing them to a NetClientTask.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef NETCLIENT_H_INCLUDED
#define NETCLIENT_H_INCLUDED

#include <map>
#include <string>
#include <com/Barrier.h>

#include "com/Dispatch.h"           // Base class (also includes List)

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class NetClientTask;                // Friend class

//----------------------------------------------------------------------------
//
// Class-
//       NetClient
//
// Purpose-
//       Process NetClientItem requests by routing them to a NetClientTask.
//
// Implementation notes-
//       Manages the creation and deletion of NetClientTask objects.
//
//----------------------------------------------------------------------------
class NetClient : public DispatchTask {
friend NetClientTask;

//----------------------------------------------------------------------------
// NetClient::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum FSM                            // Finite State Machine States
{  FSM_RESET                        // RESET, not initialized
,  FSM_READY                        // READY, operational
,  FSM_CLOSE                        // CLOSE, terminating
}; // enum FSM

enum
{  MAX_CONNECTION= 32               // Number of different hosts
}; // enum

typedef std::map<std::string,NetClientTask*>::iterator HostMapIterator;

//----------------------------------------------------------------------------
// NetClient::Attributes
//----------------------------------------------------------------------------
protected:
std::string            agent;       // The "User-Agent" property

Barrier                barrier;     // Protects the following fields
unsigned               count;       // The number of managed hosts
int                    fsm;         // Our Finite State Machine
std::map<std::string,NetClientTask*>
                       hostMap;     // The NetClientTask map
unsigned               max_count;   // The maximum number of managed hosts

//----------------------------------------------------------------------------
// NetClient::Constructors
//----------------------------------------------------------------------------
public:
   ~NetClient( void );              // Destructor
   NetClient(                       // Constructor
     const char*       agent);      // The "User-Agent" property

//----------------------------------------------------------------------------
// NetClient::Methods
//----------------------------------------------------------------------------
public:
virtual void
   remove(                          // Remove
     NetClientTask*    task);       // This NetClientTask from host array

virtual void
   shutdown( void );                // Terminate the NetClient

virtual void
   work(                            // Get URL
     DispatchItem*     uow);        // (Must be a NetworkItem*)
}; // class NetClient

#include "NetClientTask.h"          // Host-specific request processor
#include "NetClientItem.h"          // The NetClient DispatchItem type

#endif // NETCLIENT_H_INCLUDED
