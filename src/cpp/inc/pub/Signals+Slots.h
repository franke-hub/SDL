//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
// This work is derived from:
//       https://codereview.stackexchange.com/questions/98564/event-listener-implementation
//       stackexchange.com contributed content uses the MIT license.
//       Copyright (c) 2015 stackexchange.com.
//
//       Author: stackexchange.com user 25224, Lars. (Germany) [Creator]
//       Author: stackexchange.com user 15094, Morwenn. (Brittany) [Insights]
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/pub/signals/Signals+Slots.h
//
// Purpose-
//       Signals and Slots pattern implementation
//
// Last change date-
//       2018/01/01
//
// Implementation note-
//       The Connection, Signal, and Slot objects are thread-safe under the
//       following conditions:
//         * While running a signal handler:
//           Connection::reset() must not be invoked from the same thread.
//           No Signal modification from the same thread is allowed.
//              Signal::connect() will result in latch deadlock.
//              Signal::reset() will result in latch deadlock.
//         * Signal::emit() may run concurrently in multiple theads.
//           Ordering between threads is indeterminate.
//
//       Note that no thread switching occurs as a result of any operation.
//       In particular, Signal::emit (and SlotList::emit) cause slots to be
//       invoked on the same thread. This diminishes the utility of the
//       implementation, and reduces any real need for thread-safety. On the
//       other hand, Latch mutex operations when there is no conflict has
//       very low overhead. The mutex might just as well remain.
//
//----------------------------------------------------------------------------
#ifndef _PUB_SIGNALS_SLOTS_H_INCLUDED
#define _PUB_SIGNALS_SLOTS_H_INCLUDED

#include <functional>
#include <list>
#include <memory>
#include <mutex>                    // For std::lock_guard

#include <pub/config.h>             // For _PUB_NAMESPACE, ...
#include <pub/Latch.h>              // For SharedLatch, ExclusiveLatch

namespace _PUB_NAMESPACE::signals {
//----------------------------------------------------------------------------
// Compilation controls
//----------------------------------------------------------------------------
#ifndef _PUB_SIGNAL_SLOTS_HCDM      // If defined, Hard Core Debug Mode
#undef  _PUB_SIGNAL_SLOTS_HCDM      // (Undefined at end)
#endif

#ifndef _PUB_SIGNAL_SLOTS_DEBUG     // If defined, enable debugging methods
#undef  _PUB_SIGNAL_SLOTS_DEBUG     // (Undefined at end)
#endif

#if defined(_PUB_SIGNAL_SLOTS_HCDM) || defined(_PUB_SIGNAL_SLOTS_DEBUG)
#include <pub/Debug.h>              // For debugf
using _PUB_NAMESPACE::debugging::debugf;
using _PUB_NAMESPACE::debugging::tracef;
#endif

#ifdef  _PUB_SIGNAL_SLOTS_HCDM
#define _HCDM(x) {x}                // (Undefined at end)
#else
#define _HCDM(x) {}
#endif

#ifdef  _PUB_SIGNAL_SLOTS_DEBUG
#define _DEBUG(x) {x}               // (Undefined at end)
#else
#define _DEBUG(x) {}
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
template <typename ...Args>
class                  Signal;      // The Signal<...Args> class

template <typename ...Args>
using Slot=            std::function<void(Args...)>; // The Slot<...Args> class

namespace detail {
//----------------------------------------------------------------------------
//
// Class-
//       SlotList
//
// Purpose-
//       Define the list of Slots
//
//----------------------------------------------------------------------------
template <typename ...Args>
class SlotList {                    // List of Slots
protected:
using Slot_t=          Slot<Args...>;
using S_list_t=        std::list<Slot_t>;

SharedLatch            latch_;      // Shared/Exclusive Latch
S_list_t               s_list;      // The List of Slots

//----------------------------------------------------------------------------
// SlotList::Constructor/Destructor
//----------------------------------------------------------------------------
public:
SlotList()                          // Default constructor
:  latch_(), s_list()
{  _HCDM( tracef("SlotList(%p)::SlotList()\n", this); ) }

~SlotList()                         // Denstructor
{  _HCDM( tracef("SlotList(%p)::~SlotList()\n", this); ) }

//----------------------------------------------------------------------------
// SlotList::Methods
//----------------------------------------------------------------------------
void
   debug(const char* text="")       // Bringup debugging
{  _DEBUG(
     debugf("SlotList(%p)::debug(%s)\n", this, text);
     debugf("..%zd=s_list.size()\n", s_list.size());
     int X= 0;
     for(auto& slot:s_list) debugf("[%2d] %p\n", X++, &slot);
   )
}

void emit(Args... args)             // Emit signal, driving all Connections
{  _HCDM( tracef("SlotList(%p)::emit()\n", this); )
   std::lock_guard<decltype(latch_)> lock(latch_); // Holding shared Latch

   for(auto& slot:s_list) slot(args...);
}

typename S_list_t::iterator         // Slot position within list
   insert(Slot_t& h)                // Insert Slot into SlotList
{
   ExclusiveLatch exclusive(latch_);
   std::lock_guard<decltype(exclusive)> lock(exclusive); // Holding exclusive Latch

   typename S_list_t::iterator slot= s_list.insert(s_list.end(), h);
   _HCDM( tracef("%p= SlotList(%p)::insert(%p)\n", &(*slot), this, &h); )
   return slot;
}

void
   remove(typename S_list_t::iterator& slot) // Remove Slot from list
{  _HCDM( tracef("SlotList(%p)::remove(%p)\n", this, &(*slot)); )
   ExclusiveLatch exclusive(latch_);
   std::lock_guard<decltype(exclusive)> lock(exclusive); // Holding exclusive Latch

   s_list.erase(slot);
}
}; // class SlotList
}  // namespace detail

//----------------------------------------------------------------------------
//
// Class-
//       Connection
//
// Purpose-
//       Slot to Signal connector
//
// Implementation note-
//       A Connection contains a weak_ptr that references the Signal's
//       SlotList. Signal.connect() creates a Connection, but neither the
//       Signal nor the SlotList instances refer to it.
//
//----------------------------------------------------------------------------
template <typename ...Args>
class Connection {                  // Slot to Signal connector
protected:
using Signal_t=        Signal<Args...>;
using Slot_t=          Slot<Args...>;
using S_list_t=        std::list<Slot_t>;
using SlotList_p=      std::weak_ptr<detail::SlotList<Args...>>;

SlotList_p             list;        // Our associated SlotList
typename S_list_t::iterator
                       slot;        // Our position in the SlotList::s_list

//-----------------------------------------------------------------------
// Connection::Constructors/Destructor
//-----------------------------------------------------------------------
public:
Connection()                        // Default constructor
:  list(), slot()
{  _HCDM( tracef("Connection(%p)::Connection()\n", this); ) }

inline
Connection(Signal_t& s, Slot_t h);  // Signal+Slot constructor
// Implementation deferred. (Signal definition required)

Connection(const Connection& that) = delete; // NO copy constructor

Connection(Connection&& that)       // *MOVE* constructor
:  list(), slot()
{  _HCDM( tracef("Connection(%p)::Connection(Connection&& %p))\n", this, &that); )
   list= std::move(that.list);
   slot= std::move(that.slot);
   that.list.reset();
}

~Connection()                       // Destructor
{  _HCDM( tracef("Connection(%p)::~Connection()\n", this); )
   reset();
}

//-----------------------------------------------------------------------
// Connection::Assignment operators
//-----------------------------------------------------------------------
Connection& operator=(const Connection& that) = delete; // NO copy assignment

Connection& operator=(Connection&& that) // *MOVE* assignment
{  _HCDM( tracef("Connection(%p)::operator=(Connection&& %p))\n", this, &that); )
   reset();                         // Reset this instance
   list= std::move(that.list);
   slot= std::move(that.slot);
   that.list.reset();
   return *this;
}

//-----------------------------------------------------------------------
// Connection::debug() // Bringup debugging
//-----------------------------------------------------------------------
void debug(const char* text="")
{  _DEBUG(
     debugf("Connection(%p)::debug(%s)\n", this, text);
     auto strong_ptr= list.lock();
     void* pointer= strong_ptr.get();
     if( pointer ) debugf("..SlotList(%p) slot(%p)\n", pointer, &(*slot));
     else          debugf("..SlotList(%p) slot(%p)\n", pointer, nullptr);
   )
}

//-----------------------------------------------------------------------
// Connection::reset() // Reset content, iff extant
//-----------------------------------------------------------------------
void reset()
{  _HCDM( tracef("Connection(%p)::reset()\n", this); )
   auto strong_ptr= list.lock();
   if( strong_ptr )
     strong_ptr->remove(slot);

   list.reset();
}
}; // class Connection

//----------------------------------------------------------------------------
//
// Class-
//       Signal
//
// Purpose-
//       Implement Signals and Slots Signal pattern.
//
// Implementation note-
//       The list of slots, s_list, is implemented as a shared_ptr so that
//       there is something for the Connection to use to create a weak_ptr
//       reference that disappears when the Signal disappears.
//
//----------------------------------------------------------------------------
template <typename ...Args>
class Signal {
friend class Connection<Args...>;

public:
using Connection_t=    Connection<Args...>;
using Slot_t=          Slot<Args...>;
using S_list_t=        detail::SlotList<Args...>;
using S_list_p=        std::shared_ptr<S_list_t>;

//----------------------------------------------------------------------------
// Signal::Attributes
//----------------------------------------------------------------------------
protected:
S_list_p               s_list;      // The SlotList

//----------------------------------------------------------------------------
// Signal::Constructors/Destructor
//----------------------------------------------------------------------------
public:
   Signal()                         // Default constructor
:  s_list(std::make_shared<S_list_t>())
{  _HCDM( tracef("Signal(%p)::Signal()\n", this); ) }
   Signal(const Signal&) = delete;
   Signal& operator=(const Signal&) = delete;

   ~Signal()                        // Destructor
{  _HCDM( tracef("Signal(%p)::~Signal()\n", this); ) }

//----------------------------------------------------------------------------
// Signal::connect
//----------------------------------------------------------------------------
Connection_t
   connect(Slot_t s)                // Connect a Signal handler
{  _HCDM( tracef("Signal(%p)::connect(...)\n", this); )
   Connection_t p(*this, s);

   return p;
}

//----------------------------------------------------------------------------
// Signal::emit
//----------------------------------------------------------------------------
void emit(Args... args)             // Emit signal, driving all Connections
{  _HCDM( debug("emit"); )
   s_list->emit(args...);
}

//----------------------------------------------------------------------------
// Signal::debug
//----------------------------------------------------------------------------
void
   debug(const char* text="")       // Bringup debugging
{  _DEBUG( s_list->debug(text); ) }

//----------------------------------------------------------------------------
// Signal::reset
//----------------------------------------------------------------------------
void reset( void )                  // Reset the Signal
{  _HCDM( debug("reset"); )
   s_list= std::make_shared<S_list_t>();
}
}; // class Signal

//============================================================================
// Connection::Connection deferred implementation
//============================================================================
template <typename ...Args>
Connection<Args...>::Connection(Signal_t& s, Slot_t h) // Signal+Slot constructor
:  list(s.s_list), slot()
{  _HCDM( tracef("Connection(%p)::Connection(Signal_t&,Slot_t))\n", this); )
   slot= s.s_list->insert(h);

   _HCDM( debug("Constructor"); )
}
#undef  _HCDM
#undef  _PUB_SIGNAL_SLOTS_HCDM
#undef  _DEBUG
#undef  _PUB_SIGNAL_SLOTS_DEBUG
}  // namespace _PUB_NAMESPACE::signals
#endif  // _PUB_SIGNALS_SLOTS_H_INCLUDED
