//----------------------------------------------------------------------------
//
//       Copyright (C) 2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Signals.cpp
//
// Purpose-
//       Implement Signals.h
//
// Last change date-
//       2025/01/20
//
//----------------------------------------------------------------------------
#include <functional>               // For std::function
#include <memory>                   // For std::shared_ptr, std::weak_ptr
#include <mutex>                    // For std::lock_guard

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/List.h>               // For pub::List
#include <pub/Latch.h>              // For pub::XCL_latch, SHR_latch
#include "pub/Signals.h"            // For pub::Signals interface, implemented
#include "pub/Trace.h"              // For pub::Trace
#include "pub/utility.i"            // For conversion subroutines

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging methods
using PUB::utility::demangle;

namespace _LIBPUB_NAMESPACE {
namespace signals {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  USE_ITRACE= true                 // Use internal trace?
}; // generic enum

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Listener::Listener
//       pub::signals::Listener::~Listener
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   Listener::Listener(
     const Function&   _function)   // The Event handler function
:  function(_function)
{  }

   Listener::~Listener( void )      // Destructor
{  }

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Listener::signal
//
// Purpose-
//       Tell Listener about an Event
//
//----------------------------------------------------------------------------
void
   pub::signals::Listener::signal(  // Tell this Listener about
     Event&            event) const // This Event
{  function(event); }               // Invoke the associated function

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::ListenerList::ListenerList
//       pub::signals::ListenerList::~ListenerList
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   ListenerList::ListenerList( void ) // (Default) constructor
:  SHR(), list()
{  }

   ListenerList::~ListenerList( void ) // Destructor
{
   // Implementation note: No action is needed or can be performed here.
   // Only Connectors may delete Listeners and (since it's now being deleted,)
   // no Connectors (who only have weak_ptr references to this List) can find
   // it. On the other hand, we must not not access the ListenerList since the
   // Listeners on the list are owned by by Connectors, not us.
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::ListenerList::debug
//
// Purpose-
//       Debugging display, invoked by Signals::Signal::debug
//
//----------------------------------------------------------------------------
void
   ListenerList::debug(const char* info) const // Debugging display

{
   debugf("ListenerList::debug(%s)\n", info);

   size_t X= 0;                     // Pseudo-index
   std::lock_guard<decltype(SHR)> lock(SHR);
   for(Slot_t* slot= list.get_head(); slot; slot= slot->get_next()) {
     debugf("[%2zd] %p\n", X++, slot);
   }
   debugf("[%2zd] Listener%s\n", X, X == 1 ? "" : "s");
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::ListenerList::signal
//
// Purpose-
//       Signal Event occurance
//
// Implementation notes-
//       The signal method does not return until all Listeners are (serially)
//       driven. The Event object is passed by reference and may be modified
//       by Listeners for any (application-defined) purposes.
//
//----------------------------------------------------------------------------
void                                // (All Listeners are signaled)
   ListenerList::signal(            // Signal all Listeners about
     Event&            event) const // This Event
{  if( HCDM )
     debugf("ListenerList(%p)::signal(%s)\n", this
           , s2c(demangle(typeid(event))));

   // Livelock (an application loop) occurs if an application attempts to
   // modify the Slot (Listener) list during this loop.
   size_t X= 0;                     // Pseudo-index
   std::lock_guard<decltype(SHR)> lock(SHR); // While holding the shared Latch
   for(Slot_t* slot= list.get_head(); slot; slot= slot->get_next()) {
     if( HCDM && VERBOSE )
       debugf("[%2zd] %p %s\n", X++, slot, s2c(demangle(typeid(event))));
     slot->signal(event);           // signal the Listener
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::ListenerList::insert
//
// Purpose-
//       Add Slot_t into ListenerList
//
//----------------------------------------------------------------------------
void
   ListenerList::insert(            // Insert
     Slot_t*           slot)        // This Listener (FIFO ordering)
{
   XCL_latch XCL(SHR);              // While holding the exclusive Latch
   std::lock_guard<decltype(XCL)> lock(XCL);

   list.fifo(slot);                 // Insert the Slot
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::ListenerList::remove
//
// Purpose-
//       Remove Slot_t from ListenerList
//
//----------------------------------------------------------------------------
void
   ListenerList::remove(            // Remove
     Slot_t*           slot)        // This Listener
{
   XCL_latch XCL(SHR);              // While holding the exclusive Latch
   std::lock_guard<decltype(XCL)> lock(XCL);

   list.remove(slot, slot);         // Remove the Listener
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Connector::Connector
//       pub::signals::Connector::~Connector
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   Connector::Connector( void )     // Default constructor
:  list(), slot(nullptr)
{  }

   Connector::Connector(            // Constructor
     Strong_t&         _list,       // The ListenerList (shared_ptr reference)
     Slot_t*           _slot)       // The Listener (raw pointer)
:  list(_list), slot(_slot)         // (We only keep a weak_ptr<List_t>)
{  }

   Connector::Connector(            // MOVE constructor (resets source)
     Connector&&       that)
:  list(), slot(nullptr)
{
   list= ::std::move(that.list);
   slot= ::std::move(that.slot);
   that.list.reset();               // (Prevent duplicate remove)
   that.slot= nullptr;              // (Prevent duplicate delete)
}

   Connector::~Connector( void )    // Destructor
{  reset(); }

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Connector::operator=(Connector&&)
//
// Purpose-
//       Move assignment
//
//----------------------------------------------------------------------------
Connector&
   Connector::operator=(Connector&& that) // MOVE assignment
{
   reset();
   list= ::std::move(that.list);
   slot= ::std::move(that.slot);
   that.list.reset();               // (Prevent duplicate remove)
   that.slot= nullptr;              // (Prevent duplicate delete)
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Connector::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Connector::debug(const char* info) const
{
   debugf("Connector(%p)::debug(%s) lock_state(%s) Listener(%p)\n", this, info
         , list.lock() ? "connected" : "reset", slot);
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Connector::disconnect
//
// Purpose-
//       Alias for Connector::reset
//
//----------------------------------------------------------------------------
void
   Connector::disconnect( void )    // Disconnect this Connector
{  reset(); }

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Connector::reset
//
// Purpose-
//       Forget the Signal/Function association.
//
//----------------------------------------------------------------------------
void
   Connector::reset( void )         // Reset this Connector
{
   auto locked_list= list.lock();
   if( locked_list )
     locked_list->remove(slot);

   delete slot;                     // Delete the slot
   list.reset();                    // (Prevent duplicate remove)
   slot= nullptr;                   // (Prevent duplicate delete)
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Signal::Signal
//       pub::signals::Signal::~Signal
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   Signal::Signal( void )           // Constructor
:  list(std::make_shared<ListenerList>())
{  }

   Signal::~Signal( void )          // Destructor
{  }

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Signal::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Signal::debug(const char* info) const // Debugging display
{
   debugf("Signal(%p)::debug(%s)\n", this, info);
   list->debug(info);
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Signal::connect
//
// Purpose-
//       Connect a Signal and a Listener
//
// Implementation note-
//       The resultant (move copied) Connector contains (a newly created)
//       Listener, which contains the actual Event handler logic.
//
//       Use Connector method disconnect, reset, or the destructor
//       to destroy this Signal/Listener connection.
//
//       The Signal and Listener are loosely connected. Invoking Signal::reset
//       or Signal::~Signal disconnects any and all associated Listeners.
//
//----------------------------------------------------------------------------
Connector                           // The Signal/Function connector
   Signal::connect(                 // Connect a Signal Event handler
     const Function&   function)    // The Signal Event handler function
{
   Listener* slot= new Listener(function);

   list->insert(slot);
   Connector connector(list, slot);
   return connector;
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Signal::reset
//
// Purpose-
//       Reset the Signal, removing all Listeners
//
 //----------------------------------------------------------------------------
void
   Signal::reset( void )            // Reset the Signal, removing all Listeners
{  list= std::make_shared<ListenerList>(); } // Reset the ListenerList

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Signal::signal
//
// Purpose-
//       Serially invoke each connected Listener
//
//----------------------------------------------------------------------------
void
   Signal::signal(                  // Serially invoke each connected Listener
     Event&            event) const // Using this Event (parameter)
{  list->signal(event); }
}  // namespace signals
}  // namespace _LIBPUB_NAMESPACE
