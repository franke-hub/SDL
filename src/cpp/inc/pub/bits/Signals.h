//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       bits/Signals.h
//
// Purpose-
//       Signals __detail, not part of external interface
//
// Last change date-
//       2024/01/23
//
// Implementation note-
//       This include is private and only included from ../Signals.h.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_BITS_SIGNALS_H_INCLUDED
#define _LIBPUB_BITS_SIGNALS_H_INCLUDED

namespace __detail {
//----------------------------------------------------------------------------
//
// Class-
//       signals::__detail::Listener
//
// Purpose-
//       Listener descriptor, contains a std::function<void(Event&)> object.
//
// Implementation notes-
//       A Listener is often called a Slot in the literature.
//
//----------------------------------------------------------------------------
template<typename Event>
class Listener : public List<Listener<Event>>::Link { // Listener descriptor
//----------------------------------------------------------------------------
// signals::__detail::Listener::Attribute
//----------------------------------------------------------------------------
protected:
typedef std::function<void(Event&)> Function; // Function<Event&> class

const Function         function;   // The Event handler function

//----------------------------------------------------------------------------
// signals::__detail::Listener::Constructor
//----------------------------------------------------------------------------
public:
   Listener(                        // Constructor
     const Function&   _function)   // The Event handler function
:  function(_function)
{  }

//----------------------------------------------------------------------------
// signals::__detail::Listener::Destructor
//----------------------------------------------------------------------------
   ~Listener( void )                // Destructor
{  }

//----------------------------------------------------------------------------
// signals::__detail::Listener::signal
//----------------------------------------------------------------------------
void
   signal(                          // Tell this Listener about
     Event&            event) const // This (application defined) Event
{  function(event); }               // Invoke the associated function
}; // class Listener

//----------------------------------------------------------------------------
//
// Class-
//       signals::__detail::ListenerList
//
// Purpose-
//       The List<Listener<Event>> container, with locking controls
//
// Implementation notes-
//       Locking controls prevent an application from modifying a ListenerList
//       while it's being traversed by the signal method. This changes an
//       otherwise unpredicable result into a predicable one: application
//       livelock. The XCL_latch cannot be obtained while a SHR_latch exists.
//
//----------------------------------------------------------------------------
template<typename Event>
class ListenerList {                // The List of Listeners (container)
//----------------------------------------------------------------------------
// signals::__detail::ListenerList::Attributes
//----------------------------------------------------------------------------
protected:
typedef Listener<Event>Slot_t;      // The Listener class alias

mutable SHR_latch      SHR;         // Protects the List of Listeners
List<Slot_t>           list;        // The actual List of Listeners

//----------------------------------------------------------------------------
// signals::__detail::ListenerList::Constructor
//----------------------------------------------------------------------------
public:
   ListenerList( void )             // Default constructor
:  SHR(), list()
{  }

//----------------------------------------------------------------------------
// signals::__detail::ListenerList::Destructor
//----------------------------------------------------------------------------
   ~ListenerList( void )            // Destructor
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
//       signals::__detail::ListenerList::debug
//
// Purpose-
//       Debugging display, invoked by Signals::Signal::debug
//
//----------------------------------------------------------------------------
void
   debug( void )                    // Debugging display
{
   size_t X= 0;                     // Pseudo-index
   std::lock_guard<decltype(SHR)> lock(SHR);
   for(Slot_t* slot= list.get_head(); slot; slot= slot->get_next()) {
     printf("[%2zd] %p\n", X++, slot);
   }
   printf("[%2zd] Listener%s\n", X, X == 1 ? "" : "s");
}

//----------------------------------------------------------------------------
//
// Method-
//       signals::__detail::ListenerList::signal
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
   signal(                          // Signal all Listeners about
     Event&            event) const // This Event
{
   // Livelock (an application loop) occurs if an application attempts to
   // modify the Slot (Listener) list during this loop.
   std::lock_guard<decltype(SHR)> lock(SHR); // While holding the shared Latch
   for(Slot_t* slot= list.get_head(); slot; slot= slot->get_next())
     slot->signal(event);           // signal the Listener
}

//----------------------------------------------------------------------------
// signals::__detail::ListenerList::insert
//----------------------------------------------------------------------------
void
   insert(                          // Insert
     Slot_t*           slot)        // This Listener (FIFO ordering)
{
   XCL_latch XCL(SHR);              // While holding the exclusive Latch
   std::lock_guard<decltype(XCL)> lock(XCL);

   list.fifo(slot);                 // Insert the Slot
}

//----------------------------------------------------------------------------
// signals::__detail::ListenerList::remove
//----------------------------------------------------------------------------
void
   remove(                          // Remove
     Slot_t*           slot)        // This Listener
{
   XCL_latch XCL(SHR);              // While holding the exclusive Latch
   std::lock_guard<decltype(XCL)> lock(XCL);

   list.remove(slot, slot);         // Remove the Listener
}
}; // class ListenerList
}  // namespace __detail
#endif // _PUB_BITS_SIGNALS_H_INCLUDED
