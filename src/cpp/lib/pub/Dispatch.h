//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Dispatch.h
//
// Purpose-
//       Work dispatcher, including local definitions.
//
// Last change date-
//       2021/07/09
//
//----------------------------------------------------------------------------
#include <assert.h>                 // For assert
#include <mutex>                    // For std::lock_guard

#include <pub/Clock.h>              // DispatchTTL completion time
#include <pub/Debug.h>              // For debugging
#include <pub/Latch.h>              // dispatch::Timers mutex
#include <pub/List.h>               // dispatch::Task itemList
#include <pub/Named.h>              // dispatch::Timers is a Named Thread
#include <pub/Semaphore.h>          // dispatch::Timers event
#include <pub/Worker.h>             // dispatch::Task base class

#include "pub/Dispatch.h"           // Include visible class definitions
using namespace _PUB_NAMESPACE::debugging; // Enable debugging functions

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef logf
#define logf traceh                 // Alias for trace w/header
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <pub/ifmacro.h>

namespace _PUB_NAMESPACE::dispatch {
//----------------------------------------------------------------------------
//
// Class-
//       DispatchTTL
//
// Purpose-
//       Dispatch Timer Thread Link: Keep track of delay request.
//
//----------------------------------------------------------------------------
class DispatchTTL : public List<DispatchTTL>::Link {
//----------------------------------------------------------------------------
// DispatchTTL::Attributes
//----------------------------------------------------------------------------
public:
Clock const            time;        // The proposed completion time
Item* const            item;        // The work Item

//----------------------------------------------------------------------------
// DispatchTTL::Constructors
//----------------------------------------------------------------------------
public:
   ~DispatchTTL( void ) {}          // Destructor
   DispatchTTL(                     // Constructor
     Clock&            time,        // Completion time
     Item*             item)        // WorkUnit object
:  List<DispatchTTL>::Link(), time(time), item(item) {}
}; // class DispatchTTL

//----------------------------------------------------------------------------
//
// Class-
//       Timers
//
// Purpose-
//       Handle time delay requests.
//
//----------------------------------------------------------------------------
class Timers : public Thread, public Named {
//----------------------------------------------------------------------------
// Timers::Attributes
//----------------------------------------------------------------------------
protected:
Semaphore              event;       // Synchronization event object
List<DispatchTTL>      list;        // Ordered list of pending events
Latch                  mutex;       // Synchronization mutex
bool                   operational; // TRUE iff operational

//----------------------------------------------------------------------------
// Timers::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Timers( void ) {}               // Destructor
   Timers( void )                   // Constructor
:  Thread(), Named("DispatchTime")
,  event(), list(), mutex(), operational(true)
{  start(); }

//----------------------------------------------------------------------------
// Timers::Methods
//----------------------------------------------------------------------------
public:
void
   cancel(                          // Cancel
     void*             token)       // This timer event
{
   std::lock_guard<decltype(mutex)> lock(mutex);

   DispatchTTL* link= list.get_head();
   while( link ) {
     if( (void*)link == token ) {
       list.remove(link, link);
       link->item->post(Item::CC_PURGE);
       delete link;
       break;
     }
   }
}

void*                               // Cancellation token
   delay(                           // Delay for
     double            seconds,     // This many seconds, then
     Item*             item)        // Complete this work Item
{
   if( seconds < 0.015625 ) {       // If interval too short
     item->post();
     return nullptr;
   }

   Clock time(Clock::now() + seconds);
   DispatchTTL* link= new DispatchTTL(time, item);

   std::lock_guard<decltype(mutex)> lock(mutex);

   if( !operational ) {
     item->post(Item::CC_PURGE);
     delete link;
     return nullptr;
   }

   DispatchTTL* after= nullptr;
   DispatchTTL* work= list.get_head();
   while( work ) {
     if( link->time < work->time )
       break;

     after= work;
     work= work->get_next();
   }

   list.insert(after, link, link);
   if( after == nullptr )           // If new head of list
     event.post();                  // Use the new timeout

   return link;
}

virtual void                        // Operate the Thread
   run()
{
   IFHCDM( traceh("dispatch::Timers running..."); )

   while( operational ) {
     double delay= 60.0;            // Minimum wait delay (seconds)

     {{{{
       std::lock_guard<decltype(mutex)> lock(mutex);

       // Drive all expired timers
       DispatchTTL* link= list.get_head();
       while( link ) {
         delay= link->time.get() - Clock::now();
         if( delay > 0.015625 ) {
           if( delay > 60.0 )
             delay= 60.0;

           break;
         }

         list.remove(link, link);
         link->item->post();
         delete link;

         link= list.get_head();
       }
     }}}}

     event.wait(delay);
   }

   // Non-operational. Purge all timers before we go.
   std::lock_guard<decltype(mutex)> lock(mutex);

   DispatchTTL* link= list.get_head();
   while( link ) {
     list.remove(link, link);
     link->item->post(Item::CC_PURGE);
     delete link;

     link= list.get_head();
   }

   IFHCDM( traceh("dispatch::Timers ...terminated"); )
}

virtual void
   stop( void )                     // Terminate the Thread
{
   std::lock_guard<decltype(mutex)> lock(mutex);

   operational= false;
   event.post();
}
}; // class Timers
}  // namespace _PUB_NAMESPACE::dispatch
