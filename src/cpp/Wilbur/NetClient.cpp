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
//       NetClient.cpp
//
// Purpose-
//       NetClient (HTTP request router) implementation methods.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       If too many current connections, must DEFER request.
//       (The current implementation returns an error.)
//
//----------------------------------------------------------------------------
#include <map>
#include <new>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Barrier.h>

#include "Common.h"

#include "NetClient.h"

using std::map;
using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class NetClientTask;
class NetClientDone;

//----------------------------------------------------------------------------
//
// Subroutine-
//       shouldNotOccur
//
// Purpose-
//       Handle "Should Not Occur" condition
//
//----------------------------------------------------------------------------
static void
   shouldNotOccur(                  // Handle "Should Not Occur" condition
     int               line)        // At this line number
{
   debugf("%d %s SHOULD NOT OCCUR\n", line, __FILE__);
}

//----------------------------------------------------------------------------
//
// Method-
//       NetClient::~NetClient
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   NetClient::~NetClient( void )    // Destructor
{
   IFSCDM( logf("NetClient(%p)::~NetClient()\n", this); )

   shutdown();
}

//----------------------------------------------------------------------------
//
// Method-
//       NetClient::NetClient
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   NetClient::NetClient(            // Constructor
     const char*       agent)       // The "User-Agent" property
:  DispatchTask()
,  agent(agent)
,  barrier()
,  count(0)
,  fsm(FSM_RESET)
,  hostMap()
,  max_count(0)
{
   IFSCDM( logf("NetClient(%p)::NetClient(%s)\n", this, agent); )

   fsm= FSM_READY;
}

//----------------------------------------------------------------------------
//
// Method-
//       NetClient::remove
//
// Purpose-
//       Remove NetClientTask from the host map.
//
// Implementation notes-
//       Called with NetClient.barrier held.
//
//----------------------------------------------------------------------------
void
   NetClient::remove(               // Remove
     NetClientTask*    task)        // This NetClientTask from hostMap
{
   IFSCDM( logf("NetClient(%p)::remove(%p)...\n", this, task); )

   AutoBarrier lock(barrier);

   try {
     hostMap.erase(task->client);
     count--;
   } catch(...) {
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       NetClient::shutdown
//
// Purpose-
//       Terminate network processing.
//
//----------------------------------------------------------------------------
void
   NetClient::shutdown( void )      // Terminate processing
{
   logf("NetClient(%p)::shutdown()\n", this);

   Common& common= *Common::get();  // Our Common object

   {{{{
     AutoBarrier lock(barrier);
     fsm= FSM_CLOSE;

     for(HostMapIterator it= hostMap.begin(); it != hostMap.end(); it++)
     {
       NetClientTask* task= it->second;
       DispatchItem*  item= new NetClientItem();
       item->setFC(NetClientItem::FC_CLOSE);
       item->setDone(nullptr);
       common.dispatcher.enqueue(task, item);
     }

     hostMap.clear();
     count= 0;
   }}}}

   // Reset the NetClientTask
   if( DispatchTask::fsm != DispatchTask::FSM_RESET )
   {
     DispatchWait wait;             // DispatchWait object
     DispatchItem item(DispatchItem::FC_RESET, &wait);
     Common::get()->dispatcher.enqueue(this, &item);
     wait.wait();
   }

   fsm= FSM_RESET;
}

//----------------------------------------------------------------------------
//
// Method-
//       NetClient::work
//
// Purpose-
//       Process work Item [Route request]
//
//----------------------------------------------------------------------------
void
   NetClient::work(                 // Process Work
     DispatchItem*     item)        // The work Item
{
   IFSCDM( logf("NetClient(%p)::work(%p)...\n", this, item); )

   Common* common= Common::get();

   NetClientItem* netClientItem= dynamic_cast<NetClientItem*>(item);
   if( netClientItem == NULL )      // If not a NetClientItem
   {
     shouldNotOccur(__LINE__);      // Handle "Should Not Occur" condition
     item->post(item->CC_ERROR);
     return;
   }

   AutoBarrier lock(barrier);
   if( fsm != FSM_READY )
   {
     shouldNotOccur(__LINE__);      // Handle "Should Not Occur" condition
     item->post(item->CC_ERROR);
     return;
   }

   try {
     std::string client= NetClientTask::getClient(netClientItem);
     NetClientTask* task;
     HostMapIterator it= hostMap.find(client);
     if( it == hostMap.end() )
     {
       if( item->getFC() != item->FC_VALID )
       {
         shouldNotOccur(__LINE__); // Handle "Should Not Occur" condition
         item->post(item->CC_ERROR);
       }

       if( count >= MAX_CONNECTION ) // If too many connections already
       {
         shouldNotOccur(__LINE__); // Handle "Should Not Occur" condition
         item->post(item->CC_ERROR);
       }

       task= new NetClientTask(this, netClientItem);
       hostMap[client]= task;
       count++;
     }
     else
     {
       task= it->second;
       if( netClientItem->getFC() == NetClientItem::FC_TIMER )
       {
         IFHCDM( logf("%4d NetClient remove(%s)\n", __LINE__, client.c_str()); )
         if( task->fsm == NetClientTask::FSM_TIMER )
         {
           hostMap.erase(task->client);
           count--;
         }
       }
       else
         if( task->fsm == NetClientTask::FSM_TIMER )
         {
           IFHCDM( logf("%4d NetClient ready(%s)\n", __LINE__, client.c_str()); )
           task->setFSM(NetClientTask::FSM_READY);
         }

       common->dispatcher.enqueue(task, netClientItem);
     }
   } catch(const char* X) {
     logf("NetClient::work catch(const char(%s))\n", X);
     shouldNotOccur(__LINE__);      // Handle "Should Not Occur" condition
     item->post(item->CC_ERROR);
   } catch(std::exception& X) {
     logf("NetClient::work catch(exception.what(%s))\n", X.what());
     shouldNotOccur(__LINE__);      // Handle "Should Not Occur" condition
     item->post(item->CC_ERROR);
   } catch(...) {
     logf("NetClient::work catch(...)\n");
     shouldNotOccur(__LINE__);      // Handle "Should Not Occur" condition
     item->post(item->CC_ERROR);
   }

   IFSCDM( logf("...NetClient(%p)::work(%p)\n", this, item); )
}

