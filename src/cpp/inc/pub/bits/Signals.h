//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
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
//       2022/09/02
//
// Implementation note-
//       This include is private. It requires macros defined in ../Signals.h.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_BITS_SIGNALS_H_INCLUDED
#define _LIBPUB_BITS_SIGNALS_H_INCLUDED

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace signals::__detail {
//----------------------------------------------------------------------------
//
// Class-
//       signals::__detail::Listener
//
// Purpose-
//       Listener descriptor, contains a std::function<void(Event&)> object.
//
// Implementation notes-
//       In ListenerList, this class is aliased as a Slot_t.
//
//----------------------------------------------------------------------------
template<typename Event>
class Listener : public List<Listener<Event>>::Link { // Listener descriptor
//----------------------------------------------------------------------------
// signals::__detail::Listener::Attribute
//----------------------------------------------------------------------------
protected:
typedef std::function<void(Event&)>
                       Function;   // Function<Event> class

const Function         function;   // The Event handler function

//----------------------------------------------------------------------------
// signals::__detail::Listener::Constructor
//----------------------------------------------------------------------------
public:
   Listener(                        // Constructor
     const Function&   _function)   // The Event handler function
:  function(_function)
{  if( pub_hcdm )
     debugf("Listener(%p.%zd)::Listener(%p.%zd)\n"
           , this, sizeof(*this), &_function, sizeof(_function));
}

//----------------------------------------------------------------------------
// signals::__detail::Listener::Destructor
//----------------------------------------------------------------------------
   ~Listener( void )                // Destructor
{  if( pub_hcdm ) debugf("Listener(%p)::~Listener\n", this); }

//----------------------------------------------------------------------------
// signals::__detail::Listener::signal
//----------------------------------------------------------------------------
void
   signal(                          // Signal this Listener about
     Event&            event) const // This (application defined) Event
{  if( pub_hcdm ) debugf("Listener(%p)::signal(%p)\n", this, &event);

   function(event);
}
}; // class Listener

//----------------------------------------------------------------------------
//
// Class-
//       signals::__detail::ListenerList
//
// Purpose-
//       The List<Listener<Event>> container, with locking controls
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
{  if( pub_hcdm ) debugf("ListenerList(%p)::ListenerList\n", this); }

//----------------------------------------------------------------------------
// signals::__detail::ListenerList::Destructor
//----------------------------------------------------------------------------
   ~ListenerList( void )            // Destructor
{  if( pub_hcdm ) debugf("ListenerList(%p)::~ListenerList\n", this);

   // Implementation note: The ListenerList may contain Listeners. Only the
   // Connector may delete these objects and it can't find the ListenerList.
   // We, on the other hand, can no longer access the ListenerList because it
   // may contain deleted elements. So, no action is wanted here.
}

//----------------------------------------------------------------------------
// signals::__detail::ListenerList::debug
//----------------------------------------------------------------------------
void
   debug( void )                    // Debugging display
{
   size_t X= 0;
   std::lock_guard<decltype(SHR)> lock(SHR); // While holding the shared Latch
   for(Slot_t* slot= list.get_head(); slot; slot= slot->get_next() ) {
     debugf("[%2zd] %p\n", X++, slot); // (Note that X is incremented here)
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       signals::__detail::ListenerList::signal
//
// Purpose-
//       Signal Event occurance
//
//----------------------------------------------------------------------------
void                                // (All Listeners are signaled)
   signal(                          // Signal all Listeners about
     Event&            event) const // This Event
{  if( pub_hcdm ) debugf("ListenerList(%p)::signal(%p)\n", this, &event);

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
{  if( pub_hcdm ) debugf("ListenerList(%p)::insert(%p)\n", this, slot);

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
{  if( pub_hcdm ) debugf("ListenerList(%p)::remove(%p)\n", this, slot);

   XCL_latch XCL(SHR);              // While holding the exclusive Latch
   std::lock_guard<decltype(XCL)> lock(XCL);

   list.remove(slot, slot);         // Remove the Listener
}
}; // class ListenerList
}  // namespace signals::__detail
_LIBPUB_END_NAMESPACE
#endif // _PUB_BITS_SIGNALS_H_INCLUDED
