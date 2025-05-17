//----------------------------------------------------------------------------
//
//       Copyright (C) 2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Signals.cpp
//
// Purpose-
//       Implement Signals.h
//
// Last change date-
//       2025/01/22
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
//       pub::signals::Slot::Slot
//       pub::signals::Slot::~Slot
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   Slot::Slot(                      // Constructor
     const Function&   _function)   // The Event handler function
:  function(_function)
{  }

   Slot::~Slot( void )              // Destructor
{  }

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Slot::signal
//
// Purpose-
//       Tell Slot about an Event
//
//----------------------------------------------------------------------------
void
   pub::signals::Slot::signal(      // Tell this Slot about
     Event_t&          event) const // This Event
{  if( HCDM )
     debugf("Slot(%p)::signal(%s)\n", this, s2c(demangle(typeid(event))));

   function(event);                 // Invoke the associated function
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::SlotList::SlotList
//       pub::signals::SlotList::~SlotList
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   SlotList::SlotList( void )       // (Default) constructor
:  SHR(), list()
{  }

   SlotList::~SlotList( void )      // Destructor
{
   // Implementation note: No action is needed or can be performed here.
   // Only Connectors may delete Slots and (since it's now being deleted,)
   // no Connectors (who only have weak_ptr references to this List) can find
   // it. On the other hand, we must not not access the SlotList since the
   // Slots on the list are owned by by Connectors, not us.
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::SlotList::debug
//
// Purpose-
//       Debugging display, invoked by Signals::Signal::debug
//
//----------------------------------------------------------------------------
void
   SlotList::debug(const char* info) const // Debugging display

{
   debugf("SlotList::debug(%s)\n", info);

   size_t X= 0;                     // Pseudo-index
   std::lock_guard<decltype(SHR)> lock(SHR);
   for(Slot_t* slot= list.get_head(); slot; slot= slot->get_next()) {
     debugf("[%2zd] %p\n", X++, slot);
   }
   debugf("[%2zd] Slot%s\n", X, X == 1 ? "" : "s");
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::SlotList::signal
//
// Purpose-
//       Signal Event occurance
//
// Implementation notes-
//       The signal method does not return until all Slots are (serially)
//       driven. The Event object is passed by reference and may be modified
//       by Slots for any (application-defined) purposes.
//
//----------------------------------------------------------------------------
void
   SlotList::signal(                // Signal all Slots with
     Event_t&          event) const // This Event
{  if( HCDM )
     debugf("SlotList(%p)::signal(%s)\n", this, s2c(demangle(typeid(event))));

   // Livelock (an application loop) occurs if an application attempts to
   // modify the SlotList during this loop.
   size_t X= 0;                     // Pseudo-index
   std::lock_guard<decltype(SHR)> lock(SHR); // While holding the shared Latch
   for(Slot_t* slot= list.get_head(); slot; slot= slot->get_next()) {
     if( HCDM && VERBOSE )
       debugf("[%2zd] %p %s\n", X++, slot, s2c(demangle(typeid(event))));
     slot->signal(event);           // signal the Slot
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::SlotList::insert
//
// Purpose-
//       Add Slot_t into SlotList
//
//----------------------------------------------------------------------------
void
   SlotList::insert(                // Insert
     Slot_t*           slot)        // This Slot (FIFO ordering)
{
   XCL_latch XCL(SHR);              // While holding the exclusive Latch
   std::lock_guard<decltype(XCL)> lock(XCL);

   list.fifo(slot);                 // Insert the Slot
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::SlotList::remove
//
// Purpose-
//       Remove Slot_t from SlotList
//
//----------------------------------------------------------------------------
void
   SlotList::remove(                // Remove
     Slot_t*           slot)        // This Slot
{
   XCL_latch XCL(SHR);              // While holding the exclusive Latch
   std::lock_guard<decltype(XCL)> lock(XCL);

   list.remove(slot, slot);         // Remove the Slot
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
     Strong_t&         _list,       // The SlotList (shared_ptr reference)
     Slot_t*           _slot)       // The Slot     (raw pointer)
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
   debugf("Connector(%p)::debug(%s) lock_state(%s) Slot(%p)\n", this, info
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
:  list(std::make_shared<SlotList>())
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
//       Connect a Signal and a Slot
//
// Implementation note-
//       The resultant (move copied) Connector contains (a newly created)
//       Slot, which contains the actual Event handler logic.
//
//       Use Connector method disconnect, reset, or the destructor
//       to destroy this Signal/Slot connection.
//
//       The Signal and Slot are loosely connected. Invoking Signal::reset
//       or Signal::~Signal disconnects any and all associated Slots.
//
//----------------------------------------------------------------------------
Connector                           // The Signal/Function connector
   Signal::connect(                 // Connect a Signal Event handler
     const Function&   function)    // The Signal Event handler function
{
   Slot* slot= new Slot(function);

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
//       Reset the Signal, removing all Slots
//
 //----------------------------------------------------------------------------
void
   Signal::reset( void )            // Reset the Signal, removing all Slots
{  list= std::make_shared<SlotList>(); } // Reset the SlotList

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Signal::signal
//
// Purpose-
//       Serially invoke each connected Slot
//
//----------------------------------------------------------------------------
void
   Signal::signal(                  // Serially invoke each connected Slot
     Event_t&          event) const // Using this Event (parameter)
{  list->signal(event); }
}  // namespace signals
}  // namespace _LIBPUB_NAMESPACE
