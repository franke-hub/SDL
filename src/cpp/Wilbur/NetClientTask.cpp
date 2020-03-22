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
//       NetClientTask.cpp
//
// Purpose-
//       NetClientTask implementation methods.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       We assume that no connection persists long enough to worry about
//       updating robots.txt within it.
//
//       The robots.txt visit specifier is ignored.
//
// NetClient error codes-
//       401 - Forbidden (by Robots.txt)
//       601 - NetClient fault (retry)
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Barrier.h>
#include <com/Clock.h>
#include <com/Unconditional.h>

#include "Common.h"

#include "NetClientTask.h"          // Also includes NetClient.h

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
// Constants for parameterization
//----------------------------------------------------------------------------
#define KEEP_ALIVE     9.0          // KeepAlive timeout, in seconds
#define MAX_SLEEP_TIME 1.0          // Longest sleep interval while working

//----------------------------------------------------------------------------
//
// Subroutine-
//       logEvent
//
// Purpose-
//       Log NetClient access
//
// Implementation notes-
//       "ROBOTS" Access denied by Robots.txt
//       "FAILED" Unable to load from URI
//       "CACHED" Loaded cached text
//       "LOADED" Loaded text from URI
//
//       "REJECT" Internal state error
//
//----------------------------------------------------------------------------
static void
   logEvent(                        // Log NetClient access
     const NetClientItem*
                       item,        // For this Item
     const char*       result)      // Access result
{
   std::string uri= item->url.getURI();
   logf("NetClientTask: %s URL(%s)\n", result, uri.c_str());
}

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
   errorf("%4d %s SHOULD NOT OCCUR\n", line, __FILE__);
}

//----------------------------------------------------------------------------
//
// Class-
//       NetClientTask_Wait
//
// Purpose-
//       The NetClientTask KeepAlive timer event Wait object.
//
//----------------------------------------------------------------------------
class NetClientTask_Wait : public DispatchWait {
//----------------------------------------------------------------------------
// NetClientTask_Wait::Attributes
//----------------------------------------------------------------------------
protected:
NetClientTask*          task;        // Associated NetClientTask

//----------------------------------------------------------------------------
// NetClientTask_Wait::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~NetClientTask_Wait( void ) {}   // Destructor
   NetClientTask_Wait(              // Contructor
     NetClientTask*    task)        // -> NetClientTask
:  DispatchWait()
,  task(task) {}

public:
virtual void
   done(                            // Work Item completion
     DispatchItem*     item)        // With this Item
{
   IFSCDM( logf("NetClientTask_Wait(%p)::done()\n", this); )

   task->timer();                   // Note: May delete Task and this
}
}; // class NetClientTask_Wait

//----------------------------------------------------------------------------
//
// Method-
//       NetClientTask::~NetClientTask
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NetClientTask::~NetClientTask( void ) // Destructor
{
   IFSCDM( logf("NetClientTask(%p)::~NetClientTask()\n", this); )

   if( kaItem != NULL )
   {
     delete kaItem;
     kaItem= NULL;
   }

   if( kaWait != NULL )
   {
     delete kaWait;
     kaWait= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       NetClientTask::NetClientTask
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NetClientTask::NetClientTask(    // Constructor
     NetClient*        owner,       // Owner NetClient
     NetClientItem*    item)        // First work Item
:  DispatchTask()
,  owner(owner)
,  client(getClient(item))
,  barrier()
,  fsm(FSM_RESET)
,  cached()
,  robots()
,  alive(0.0)
,  clock(0.0)
,  token(NULL)
,  kaItem(NULL)
,  kaWait(NULL)
{
   IFSCDM(
     logf("NetClientTask(%p)::NetClientTask(%p,%s)\n", this,
          owner, client.c_str());
   )

   // Get robots.txt
   string s= client + "/robots.txt";
// debugf("%4d NetClientTask(%p) robots(%s)\n", __LINE__, this, s.c_str());

   cached.setRequestProperty("Connection", "Keep-Alive");
   cached.setNullTimeout(7*24*3600);
   cached.open(s.c_str());
   robots.open(owner->agent.c_str(), cached);
   cached.setNullTimeout(0);

   // Initialize and start keep-alive timer
   kaItem= new NetClientItem();
   kaWait= new NetClientTask_Wait(this);
   kaItem->setFC(NetClientItem::FC_TIMER);
   kaItem->setDone(kaWait);
   kaItem->url= client;

   setFSM(FSM_READY);               // Go into READY state
   token= Common::get()->dispatcher.delay(KEEP_ALIVE, kaItem);

   // Handle initial work Item
   work(item);
}

//----------------------------------------------------------------------------
//
// Method-
//       NetClientTask::getClient
//
// Purpose-
//       Can this Task handle the specified Item?
//
//----------------------------------------------------------------------------
std::string                         // The associated protocol, host, port
   NetClientTask::getClient(        // Get associated protocol, host, port
     NetClientItem*    item)        // For this NetClientItem
{
   Url& url= item->url;
   string host= url.getUserInfo();
   if( host == "" )
     host= url.getHost();
   else
     host += "@" + url.getHost();

   int port= url.getPort();
   if( port < 0 )
     port= url.getDefaultPort();

   return  url.getProtocol() + "://" + host + ":" + std::to_string(port);
}

//----------------------------------------------------------------------------
//
// Method-
//       NetClientTask::setFSM
//
// Purpose-
//       Set the FSM.
//
//----------------------------------------------------------------------------
void
   NetClientTask::setFSM(           // Update the FSM
     int               fsm)         // To this state
{
   IFSCDM( logf("NetClientTask(%p)::setFSM(%d=>%d)\n", this, this->fsm, fsm); )

   this->fsm= fsm;
}

//----------------------------------------------------------------------------
//
// Method-
//       NetClientTask::timer
//
// Purpose-
//       Handle KeepAlive timer event.
//
// Implementation note-
//       The NetClientTask is ONLY deleted here after a timer expiration.
//
//----------------------------------------------------------------------------
void
   NetClientTask::timer( void )     // Handle KeepAlive timer event
{
   IFSCDM( logf("NetClientTask(%p)::timer() %d\n", this, fsm); )

   Common& common= *Common::get();  // Our Common

   // Update the Task state
   switch(fsm)
   {
     case FSM_CLOSE:
     case FSM_RESET:
     case FSM_TIMER:
       {{{{
         if( kaItem->getFC() != DispatchItem::FC_RESET )
         {
           kaItem->setFC(DispatchItem::FC_RESET);
           Common::get()->dispatcher.enqueue(this, kaItem);
           return;
         }

         delete this;
         return;
       }}}}

     case FSM_READY:
       {{{{
         Clock now;

         if( ((double)now - (double)alive) < KEEP_ALIVE )
         {
           //---------------------------------------------------------------
           // Update the delay time. (The connection remains active.)
           double delay= KEEP_ALIVE - (now - alive);
           token= common.dispatcher.delay(delay, kaItem);
           break;
         }

         //-------------------------------------------------------------------
         // The KeepAlive timer has expired.
         // We forward the kaItem to the NetClient Task
         setFSM(FSM_TIMER);
         common.dispatcher.enqueue(owner, kaItem);
         break;
       }}}}

     default:
       shouldNotOccur(__LINE__);    // Handle "Should Not Occur" condition
       break;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       NetClientTask::work
//
// Purpose-
//       Process a work Item
//
//----------------------------------------------------------------------------
void
   NetClientTask::work(             // Process
     DispatchItem*     item)        // This NetClientItem
{
   IFSCDM( logf("NetClientTask(%p)::work(%p)\n", this, item); )

   NetClientItem* netClientItem= dynamic_cast<NetClientItem*>(item);
   if( netClientItem == NULL )
   {
     shouldNotOccur(__LINE__);      // Handle "Should Not Occur" condition
     item->post(item->CC_ERROR);
     return;
   }

   //-------------------------------------------------------------------------
   // Initialize response data
   netClientItem->rc= (-1);
   netClientItem->data.reset();

   //-------------------------------------------------------------------------
   // Initial state checks
   switch( netClientItem->getFC() )
   {
     case NetClientItem::FC_CLOSE:
       fsm= FSM_CLOSE;
       Common::get()->dispatcher.cancel(token);
       item->post(item->CC_NORMAL);
       return;

     case NetClientItem::FC_TIMER:
       item->post(item->CC_NORMAL);
       return;

     case NetClientItem::FC_VALID:
       break;

     default:
       logEvent(netClientItem, "REJECT");

       netClientItem->rc= 601;
       netClientItem->post(netClientItem->CC_INVALID_FC);
       return;
   }

   if( fsm != FSM_READY )
   {
     logEvent(netClientItem, "REJECT");

     netClientItem->rc= 601;
     netClientItem->post(netClientItem->CC_ERROR);
     return;
   }

   //-------------------------------------------------------------------------
   // Obey robots.txt access
   std::string uri= netClientItem->url.getURI();
   if( !robots.allowed(uri.c_str()) )
   {
     logEvent(netClientItem, "ROBOTS");

     netClientItem->rc= 401;          // Not authorized
     netClientItem->post(netClientItem->CC_ERROR);
     return;
   }

   //-------------------------------------------------------------------------
   // Read from CACHE
   netClientItem->rc= cached.open(uri.c_str(), TRUE);
   if( netClientItem->rc == 0 )
   {
     logEvent(netClientItem, "CACHED");
     alive= Clock::current();

     netClientItem->data= cached;
     netClientItem->post(netClientItem->CC_NORMAL);
     return;
   }

   //-------------------------------------------------------------------------
   // Obey robots.txt delay (IGNORE robots.text visit)
   Clock now;
   while( fsm == FSM_READY && robots.getDelay() > (now - clock) )
   {
     Clock next= (double)clock + robots.getDelay();

     double delay= next - now;
     if( delay > MAX_SLEEP_TIME )
       delay= MAX_SLEEP_TIME;
     Thread::sleep(delay);

     now= Clock::current();
   }

   //-------------------------------------------------------------------------
   // Secondary state check
   if( fsm != FSM_READY )
   {
     logEvent(netClientItem, "REJECT");

     netClientItem->rc= 601;
     netClientItem->post(netClientItem->CC_ERROR);
     return;
   }

   //-------------------------------------------------------------------------
   // Read from source
   alive= clock= Clock::current();

   netClientItem->rc= cached.open(uri.c_str());
   if( netClientItem->rc != 0 )
     logEvent(netClientItem, "FAILED");
   else
     logEvent(netClientItem, "LOADED");

   netClientItem->data= cached;
   netClientItem->post(netClientItem->CC_NORMAL);
}

