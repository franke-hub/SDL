//----------------------------------------------------------------------------
//
//       Copyright (C) 2023-2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Signals.h
//
// Purpose-
//       Loosely coupled event detection and event processing mechanism.
//
// Last change date-
//       2024/01/23
//
// Implementation notes-
//       Signal objects are defined in namespace pub::signals
//       This is an implemenation of the Signals and Slots, or a detached
//       Observer interface.
//
// Usage notes-
//       See related documentation: ~/doc/cpp/Signals.md for more detail.
//       Briefly, the Signal interface consists of three objects:
//         Event: The (application defined) signal Event parameter object
//         Connector: "Connects" a Signal and a Listener
//         Signal: The signal generation object
//       An important internal object exists:
//         Listener: The Event handler container
//
//       An application defines the Event, the event handler parameter.
//       Signal processing uses this Event as a template, defining the
//       Connector, Signal, and Listener objects.
//
//       A Connector "connects" a Signal to a Listener. The Listener is an
//       internal object managaged by the Signal interface which contains
//       an Event handler (function).
//
//       When an application detects an event, it invokes Signal::signal
//       which invokes currently "connected" Event handlers.
//
// Thread safety-
//       Signal objects are NOT thread-safe. Usage restrictions disallow
//       certain operations even within single threaded applications.
//
// Usage restrictions-
//       Event handlers *MUST NOT* modify Signal handler's Listener lists.
//       That is, an Event handler must not invoke Signal::connect. It also
//       must not allow Connector::reset to be invoked, either directly or
//       indirectly via Connector::disconnect or invoking a Connector's
//       destructor.
//
//       Currently, violating this restriction results in a Latch livelock,
//       an infinite application program loop.
//       This implementation may change. Your results are unpredictable.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_SIGNALS_H_INCLUDED
#define _LIBPUB_SIGNALS_H_INCLUDED

#include <functional>               // For std::function
#include <memory>                   // For std::shared_ptr, std::weak_ptr
#include <mutex>                    // For std::lock_guard
#include <stdio.h>                  // For printf

#include <pub/List.h>               // For pub::List
#include <pub/Latch.h>              // For pub::XCL_latch, SHR_latch

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace signals {
//----------------------------------------------------------------------------
// Internal classes (__detail::Listener, __detail::ListenerList)
//----------------------------------------------------------------------------
#include "bits/Signals.h"           // For internal classes

//----------------------------------------------------------------------------
//
// Typename-
//       signals::Function<Event>
//
// Purpose-
//       Define the (application-implemented) Signal Event handling function.
//
//----------------------------------------------------------------------------
//template <typename Event>
//using Function=        std::function<void(Event&)>; // Function<Event> class

//----------------------------------------------------------------------------
//
// Class-
//       signals::Connector<Event>
//
// Purpose-
//       Signal/Listener connection control.
//
// Implementation notes-
//       A Connector can be moved but cannot be copied.
//
//       When active, a connector contains a raw Listener* and a std::weak_ptr
//       to the Signal's ListenerList. This implements loose coupling between
//       to the Signal's ListenerList. The Signal and the Listener are loosely
//       connected. One does not rely on the existence of the other.
//
//----------------------------------------------------------------------------
template<typename Event>
class Connector {                   // Signal/Listener Connector
//----------------------------------------------------------------------------
// signals::Connector::Attributes
//----------------------------------------------------------------------------
protected:
typedef __detail::ListenerList<Event>         List_t;   // ListenerList type
typedef __detail::Listener<Event>             Slot_t;   // Listener type
typedef ::std::shared_ptr<List_t>             Strong_t;
typedef ::std::weak_ptr<List_t>               Weak_t;

Weak_t                 list;        // The ListenerList (weak_ptr)
Slot_t*                slot;        // The Listener (raw pointer)

//----------------------------------------------------------------------------
// signals::Connector::Constructors
//----------------------------------------------------------------------------
public:
   Connector( void )                // Default constructor
:  list(), slot(nullptr)
{  }

   Connector(                       // Constructor
     Strong_t&         _list,       // The ListenerList (shared_ptr reference)
     Slot_t*           _slot)       // The Listener (raw pointer)
:  list(_list), slot(_slot)         // (We only keep a weak_ptr<List_t>)
{  }

   Connector(const Connector&) = delete; // *NO* copy constructor

   Connector(                       // MOVE constructor (resets source)
     Connector&&       that)
:  list(), slot(nullptr)
{
   list= ::std::move(that.list);
   slot= ::std::move(that.slot);
   that.list.reset();               // (Prevent duplicate remove)
   that.slot= nullptr;              // (Prevent duplicate delete)
}

//----------------------------------------------------------------------------
// signals::Connector::Destructor
//----------------------------------------------------------------------------
   ~Connector( void )               // Destructor
{  reset(); }

//----------------------------------------------------------------------------
// signals::Connector::Operators
//----------------------------------------------------------------------------
Connector&
   operator=(Connector&& that)      // MOVE assignment (resets source)
{
   reset();
   list= ::std::move(that.list);
   slot= ::std::move(that.slot);
   that.list.reset();               // (Prevent duplicate remove)
   that.slot= nullptr;              // (Prevent duplicate delete)
   return *this;
}

// *NO* copy assignment. Applications must use connect_a= std::move(connect_b)
Connector& operator=(const Connector&) = delete;

//----------------------------------------------------------------------------
//
// Method-
//       signals::Connector::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   debug(const char* info= "")      // Debugging display
{
   printf("Connector(%p)::debug(%s) lock_state(%s) Listener(%p)\n", this, info
         , list.lock() ? "connected" : "reset", slot);
}

//----------------------------------------------------------------------------
//
// Method-
//       signals::Connector::disconnect
//
// Purpose-
//       Alias for Connector::reset
//
//----------------------------------------------------------------------------
void
   disconnect( void )               // Disconnect (reset) this Connector
{  reset(); }

//----------------------------------------------------------------------------
//
// Method-
//       signals::Connector::reset
//
// Purpose-
//       Forget the Signal/Function association.
//
//----------------------------------------------------------------------------
void
   reset( void )                    // Reset Connector
{
   auto locked_list= list.lock();
   if( locked_list )
     locked_list->remove(slot);

   delete slot;                     // Delete the slot
   list.reset();                    // (Prevent duplicate remove)
   slot= nullptr;                   // (Prevent duplicate delete)
}
}; // class signals::Connector

//----------------------------------------------------------------------------
//
// Class-
//       signals::Signal<Event>
//
// Purpose-
//       Signal descriptor
//
// Implementation notes-
//       There is *NO* copy constructor or assignment.
//       While it would be possible to implement move construction and
//       assignment, no use case has yet arisen.
//
//----------------------------------------------------------------------------
template<typename Event>
class Signal {                      // Signal descriptor
//----------------------------------------------------------------------------
// signals::Signal::Attributes
//----------------------------------------------------------------------------
protected:
typedef std::function<void(Event&)> Function; // Function<Event> class
typedef __detail::Listener<Event>   Listener; // (Shortcut)
typedef __detail::ListenerList<Event> ListenerList; // (Shortcut)

::std::shared_ptr<ListenerList>
                       list;        // The ListenerList List

//----------------------------------------------------------------------------
// signals::Signal::constructors/destructor
//----------------------------------------------------------------------------
public:
   Signal( void )                   // Constructor
:  list(std::make_shared<ListenerList>())
{  }

   Signal(const Signal&) = delete;  // *NO* copy constructor

virtual
   ~Signal( void )                  // Destructor
{  }

//----------------------------------------------------------------------------
// signals::Signal::Operators
//----------------------------------------------------------------------------
Signal& operator=(const Signal&) = delete; // *NO* Assignment operator

//----------------------------------------------------------------------------
//
// Method-
//       signals::Signal::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   debug(const char* info= "")      // Debugging display
{
   printf("Signal(%p)::debug(%s)\n", this, info);
   list->debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       signals::Signal::connect
//
// Purpose-
//       Connect a Signal and a Listener
//
// Implementation note-
//       The resultant (move copied) Connector contains (a newly created)
//       Listener, which contains the actual Event handler logic.
//
//       Use Connector<Event> method disconnect, reset, or the destructor
//       to destroy this Signal/Listener connection.
//
//       The Signal and Listener are loosely connected. Invoking Signal::reset
//       or Signal::~Signal disconnects any and all associated Listeners.
//
//----------------------------------------------------------------------------
Connector<Event>                    // The Signal/Function connector
   connect(                         // Connect a Signal Event handler
     const Function&   function)    // The Signal Event handler function
{
   Listener* slot= new Listener(function);

   list->insert(slot);
   Connector<Event> c(list, slot);
   return c;
}

//----------------------------------------------------------------------------
//
// Method-
//       signals::Signal::reset
//
// Purpose-
//       Reset the Signal, removing all Listeners
//
//----------------------------------------------------------------------------
void
   reset( void )                    // Reset the Signal, removing all Listeners
{  list= std::make_shared<ListenerList>(); } // Reset the ListenerList

//----------------------------------------------------------------------------
//
// Method-
//       signals::Signal::signal
//
// Purpose-
//       Serially invoke all connected Listeners
//
//----------------------------------------------------------------------------
void
   signal(                          // Serially invoke connected Listeners
     Event&            event) const // Using this Event (parameter)
{  list->signal(event); }
}; // class signals::Signal
}  // namespace signals
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_SIGNALS_H_INCLUDED
