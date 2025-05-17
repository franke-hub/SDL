//----------------------------------------------------------------------------
//
//       Copyright (C) 2023-2025 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Signals.h
//
// Purpose-
//       Loosely coupled event detection/processing mechanism.
//
// Last change date-
//       2025/01/22
//
// Implementation notes-
//       Signal objects are defined in namespace pub::signals
//       This is an implemenation of the "detached Observer" interface,
//       a.k.a. "Signals and Slots"
//
// Usage notes-
//       See related documentation: ~/doc/cpp/Signals.md for more detail.
//       Briefly, the Signal interface consists of three objects:
//         Event_t: The (application defined) signal parameter object
//         Connector: "Connects" a Signal and a Slot.
//         Signal: The signal generation object
//       Important internal objects exist:
//         Slot: A std::function<void(Event_t&)> container.
//         SlotList: The set of connected Slot objects.
//
//       Applications define their Event_t handler using the Signal::connect
//       method. In order to use unique parameters, the application creates
//       a struct or class publicly derived from Event_t containing these
//       additional parameters. A short example:
//       ```
//         using namespace pub::signals;
//         struct MyEvent : public Event_t { // My parameter list
//           int x,y;
//           MyEvent(int _x, int _y) : x(_x), y(_y) {}
//         };
//
//         Signal my_signal;        // My Signal
//         Connector my_connector= my_signal.connect([](Event_t& _event) {
//           MyEvent& event= static_cast<MyEvent&>(_event);
//           printf("TEST: {%d,%d}\n", event.x, event.y);
//         });
//
//         MyEvent my_event(17,19);
//         my_signal.signal(my_event);
//       ```
//
// Thread safety-
//       Signal objects are NOT thread-safe. Usage restrictions even disallow
//       certain operations even within single threaded applications.
//
// Usage restrictions-
//       We invoke Event_t handlers with a shared SlotList latch, therefore
//       Event handlers *MUST NOT* modify a Signal's SlotList.
//       That is, an Event handler must not invoke Signal::connect.
//       It also must not allow Connector::reset or Connector::disconnect to
//       be invoked, either directly or indirectly. (A Connector's destructor
//       invokes Connector::reset.)
//       Violating this restriction currently results in a Latch livelock,
//       an infinite application program loop while trying to obtain exclusive
//       SlotList access while holding shared access.
//
//       While an Event_t parameter may be modified by an Event_t handler,
//       when a Signal contains multiple active Connections, the sequence that
//       these Connections is invoked is not specified.
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
//       pub::signals::Event_t
//
// Purpose-
//       Define the pub::signals::Event_t base struct
//
// Implementation notes-
//       All application Events inherit from this struct.
//
//----------------------------------------------------------------------------
struct Event_t {                    // The signals Event base class
   Event_t( void ) = default;       // Constructor

virtual
   ~Event_t( void ) = default;      // Destructor
}; // struct Event_t

//----------------------------------------------------------------------------
//
// Class-
//       pub::signals::Slot
//
// Purpose-
//       Slot descriptor, contains a std::function<void(Event_t&)> object.
//
//----------------------------------------------------------------------------
class Slot : public List<Slot>::Link { // Slot descriptor
//----------------------------------------------------------------------------
// pub::signals::Slot::Typedefs and enumerations
//----------------------------------------------------------------------------
protected:
typedef std::function<void(Event_t&)>         Function;

//----------------------------------------------------------------------------
// pub::signals::Slot::Attributes
//----------------------------------------------------------------------------
const Function         function;    // The Event_t handler function

//----------------------------------------------------------------------------
// pub::signals::Slot::Constructor/destructor
//----------------------------------------------------------------------------
public:
   Slot(const Function&);           // Constructor

   ~Slot( void );                   // Destructor

//----------------------------------------------------------------------------
// pub::signals::Slot::signal
//----------------------------------------------------------------------------
void
   signal(Event_t&) const;          // Tell this Slot about an Event
}; // class Slot

//----------------------------------------------------------------------------
//
// Class-
//       pub::signals::SlotList
//
// Purpose-
//       The List<Slot> container, with locking controls
//
// Implementation notes-
//       Locking controls prevent an application from modifying a SlotList
//       while it's being traversed by the signal method. This changes an
//       otherwise unpredicable result into a predicable one: application
//       livelock. The XCL_latch cannot be obtained while a SHR_latch exists.
//
//----------------------------------------------------------------------------
class SlotList {                    // The List of connected Slots
//----------------------------------------------------------------------------
// pub::signals::SlotList::Attributes
//----------------------------------------------------------------------------
protected:
typedef Slot           Slot_t;      // The Slot class alias

mutable SHR_latch      SHR;         // Protects the List of Slots
List<Slot_t>           list;        // The actual List of Slots

//----------------------------------------------------------------------------
// pub::signals::SlotList::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   SlotList( void );                // Default constructor

   ~SlotList( void );               // Destructor

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
   debug(const char* info= "") const; // Debugging display

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
void                                // (All Slots are signaled)
   signal(Event_t&) const;          // Signal all Slots with Event_t

//----------------------------------------------------------------------------
// pub::signals::SlotList::insert
//----------------------------------------------------------------------------
void
   insert(Slot_t*);                 // Insert Slot (FIFO ordering)

//----------------------------------------------------------------------------
// pub::signals::SlotList::remove
//----------------------------------------------------------------------------
void
   remove(Slot_t*);                 // Remove Slot
}; // class SlotList

//----------------------------------------------------------------------------
//
// Class-
//       signals::Connector
//
// Purpose-
//       Signal/Slot connection control.
//
// Implementation notes-
//       A Connector can be moved but cannot be copied.
//
//       When active, a connector contains a raw Slot* and a std::weak_ptr
//       to the Signal's SlotList. This implements loose coupling between
//       to the Signal's SlotList. The Signal and the Slot are loosely
//       connected. One does not rely on the existence of the other.
//
//----------------------------------------------------------------------------
class Connector {                   // Signal/Slot Connector
//----------------------------------------------------------------------------
// pub::signals::Connector::Typedefs and enumerations
//----------------------------------------------------------------------------
protected:
typedef SlotList       List_t;      // SlotList type
typedef Slot           Slot_t;      // Slot type
typedef ::std::shared_ptr<List_t>   Strong_t;
typedef ::std::weak_ptr<List_t>     Weak_t;

//----------------------------------------------------------------------------
// pub::signals::Connector::Attributes
//----------------------------------------------------------------------------
Weak_t                 list;        // The SlotList (weak_ptr)
Slot_t*                slot;        // The Slot (raw pointer)

//----------------------------------------------------------------------------
// pub::signals::Connector::Constructors
//----------------------------------------------------------------------------
public:
   Connector( void );               // Default constructor

   Connector(                       // Constructor
     Strong_t&         _list,       // The SlotList (shared_ptr reference)
     Slot_t*           _slot);      // The Slot (raw pointer)

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
typedef std::function<void(Event_t&)>          Function;

//----------------------------------------------------------------------------
// pub::signals::Signal::Attributes
//----------------------------------------------------------------------------
protected:
::std::shared_ptr<SlotList>
                       list;        // The SlotList List

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
//       Connect a Signal and a Slot
//
// Implementation note-
//       The resultant (move copied) Connector contains (a newly created)
//       Slot, which contains the actual Event handler logic.
//
//       Use either of the Connector methods reset or disconnect to remove the
//       Signal/Slot connection.
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
   emit(                            // Serially invoke connected Slots
     Event_t&          event) const // Using this Event (parameter)
{  signal(event); }

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
   reset( void );                   // Reset the Signal, removing all Slots

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Signal::signal
//
// Purpose-
//       Serially invoke all connected Slots
//
//----------------------------------------------------------------------------
void
   signal(Event_t&) const;          // Serially invoke connected Slots
}; // class pub::signals::Signal
}  // namespace signals
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_SIGNALS_H_INCLUDED
