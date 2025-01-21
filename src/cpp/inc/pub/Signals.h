//----------------------------------------------------------------------------
//
//       Copyright (C) 2023-2025 Frank Eskesen.
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
//       2025/01/20
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
#include <cstdio>                   // For printf

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/List.h>               // For pub::List
#include <pub/Latch.h>              // For pub::XCL_latch, SHR_latch

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace signals {
//----------------------------------------------------------------------------
//
// Struct-
//       pub::signals::Event
//
// Purpose-
//       Define the signals::Event base struct
//
// Implementation notes-
//       All application Events inherit from this struct.
//
//----------------------------------------------------------------------------
struct Event {                      // The signals Event base class
   Event( void ) = default;         // Constructor

virtual
   ~Event( void ) = default;        // Destructor
};

//----------------------------------------------------------------------------
//
// Class-
//       pub::signals::Listener
//
// Purpose-
//       Listener descriptor, contains a std::function<void(Event&)> object.
//
// Implementation notes-
//       A Listener is often called a Slot in the literature.
//
//----------------------------------------------------------------------------
class Listener : public List<Listener>::Link { // Listener descriptor
//----------------------------------------------------------------------------
// pub::signals::Listener::Typedefs and enumerations
//----------------------------------------------------------------------------
protected:
typedef std::function<void(Event&)> Function; // Event handler function

//----------------------------------------------------------------------------
// pub::signals::Listener::Attributes
//----------------------------------------------------------------------------
const Function         function;    // The Event handler function

//----------------------------------------------------------------------------
// pub::signals::Listener::Constructor/destructor
//----------------------------------------------------------------------------
public:
   Listener(const Function&);       // Constructor

   ~Listener( void );               // Destructor

//----------------------------------------------------------------------------
// pub::signals::Listener::signal
//----------------------------------------------------------------------------
void
   signal(Event&) const;             // Tell this Listener about an Event
}; // class Listener

//----------------------------------------------------------------------------
//
// Class-
//       pub::signals::ListenerList
//
// Purpose-
//       The List<Listener> container, with locking controls
//
// Implementation notes-
//       Locking controls prevent an application from modifying a ListenerList
//       while it's being traversed by the signal method. This changes an
//       otherwise unpredicable result into a predicable one: application
//       livelock. The XCL_latch cannot be obtained while a SHR_latch exists.
//
//----------------------------------------------------------------------------
class ListenerList {                // The List of Listeners (container)
//----------------------------------------------------------------------------
// pub::signals::ListenerList::Attributes
//----------------------------------------------------------------------------
protected:
typedef Listener       Slot_t;      // The Listener class alias

mutable SHR_latch      SHR;         // Protects the List of Listeners
List<Slot_t>           list;        // The actual List of Listeners

//----------------------------------------------------------------------------
// pub::signals::ListenerList::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   ListenerList( void );            // Default constructor

   ~ListenerList( void );           // Destructor

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
   debug(const char* info= "") const; // Debugging display

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
   signal(Event&) const;            // Signal all Listeners with Event

//----------------------------------------------------------------------------
// pub::signals::ListenerList::insert
//----------------------------------------------------------------------------
void
   insert(Slot_t*);                 // Insert Listener Slot (FIFO ordering)

//----------------------------------------------------------------------------
// pub::signals::ListenerList::remove
//----------------------------------------------------------------------------
void
   remove(Slot_t*);                 // Remove Listener Slot
}; // class ListenerList

//----------------------------------------------------------------------------
//
// Class-
//       signals::Connector
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
class Connector {                   // Signal/Listener Connector
//----------------------------------------------------------------------------
// pub::signals::Connector::Typedefs and enumerations
//----------------------------------------------------------------------------
protected:
typedef ListenerList   List_t;      // ListenerList type
typedef Listener       Slot_t;      // Listener type
typedef ::std::shared_ptr<List_t>   Strong_t;
typedef ::std::weak_ptr<List_t>     Weak_t;

//----------------------------------------------------------------------------
// pub::signals::Connector::Attributes
//----------------------------------------------------------------------------
Weak_t                 list;        // The ListenerList (weak_ptr)
Slot_t*                slot;        // The Listener (raw pointer)

//----------------------------------------------------------------------------
// pub::signals::Connector::Constructors
//----------------------------------------------------------------------------
public:
   Connector( void );               // Default constructor

   Connector(                       // Constructor
     Strong_t&         _list,       // The ListenerList (shared_ptr reference)
     Slot_t*           _slot);      // The Listener (raw pointer)

   Connector(const Connector&) = delete; // *NO* copy constructor

   Connector(                       // MOVE constructor (resets source)
     Connector&&       that);

//----------------------------------------------------------------------------
// pub::signals::Connector::Destructor
//----------------------------------------------------------------------------
   ~Connector( void );              // Destructor

//----------------------------------------------------------------------------
// pub::signals::Connector::Operators
//----------------------------------------------------------------------------
Connector&
   operator=(Connector&&);          // MOVE assignment (resets source)

// *NO* copy assignment. Applications must use connect_a= std::move(connect_b)
Connector& operator=(const Connector&) = delete;

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
   debug(const char* info= "") const; // Debugging display

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
   disconnect( void );              // Disconnect (reset) this Connector

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
   reset( void );                   // Reset Connector
}; // Class pub::signals::Connector

//----------------------------------------------------------------------------
//
// Class-
//       pub::signals::Signal
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
class Signal {                      // Signal descriptor
//----------------------------------------------------------------------------
// pub::signals::Signal::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::function<void(Event&)> Function; // Event handler Function

//----------------------------------------------------------------------------
// pub::signals::Signal::Attributes
//----------------------------------------------------------------------------
protected:
::std::shared_ptr<ListenerList>
                       list;        // The ListenerList List

//----------------------------------------------------------------------------
// pub::signals::Signal::constructors/destructor
//----------------------------------------------------------------------------
public:
   Signal( void );                  // Constructor

   Signal(const Signal&) = delete;  // *NO* copy constructor

virtual
   ~Signal( void );                 // Destructor

//----------------------------------------------------------------------------
// pub::signals::Signal::Operators
//----------------------------------------------------------------------------
Signal& operator=(const Signal&) = delete; // *NO* Assignment operator

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
   debug(const char* info= "") const; // Debugging display

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
   connect(const Function&);        // Connect a Signal Event handler

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Signal::emit
//
// Purpose-
//       Alas for signal
//
//----------------------------------------------------------------------------
void
   emit(                            // Serially invoke connected Listeners
     Event&            event) const // Using this Event (parameter)
{  signal(event); }

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
   reset( void );                   // Reset the Signal, removing all Listeners

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Signal::signal
//
// Purpose-
//       Serially invoke all connected Listeners
//
//----------------------------------------------------------------------------
void
   signal(Event&) const;            // Serially invoke connected Listeners
}; // class pub::signals::Signal
}  // namespace signals
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_SIGNALS_H_INCLUDED
