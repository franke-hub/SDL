//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       detail/Signals.h
//
// Purpose-
//       Signals detail, not part of external interface
//
// Last change date-
//       2020/10/30
//
// Implementation note-
//       This include is private. It requires macros defined in ../Signals.h.
//
//----------------------------------------------------------------------------
#ifndef _PUB_DETAIL_SIGNALS_H_INCLUDED
#define _PUB_DETAIL_SIGNALS_H_INCLUDED

namespace pub::signals {
namespace detail {
//----------------------------------------------------------------------------
//
// Class-
//       pub::signals::detail::Listener
//
// Purpose-
//       Listener descriptor, contains a std::function<void(Event&)> object.
//
// Implementation notes-
//       In ListenerList, this class is aliased as a Slot_t.
//
//----------------------------------------------------------------------------
template<typename Event>
class Listener : public pub::List<Listener<Event>>::Link { // Listener descriptor
//----------------------------------------------------------------------------
// pub::Listener::Attribute
//----------------------------------------------------------------------------
protected:
typedef std::function<void(Event&)>
                       Function;   // Function<Event> class

const Function         function;   // The Event handler function

//----------------------------------------------------------------------------
// pub::signals::detail::Listener::Constructor
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
// pub::signals::detail::Listener::Destructor
//----------------------------------------------------------------------------
   ~Listener( void )                // Destructor
{  if( pub_hcdm ) debugf("Listener(%p)::~Listener\n", this); }

//----------------------------------------------------------------------------
// pub::signals::detail::Listener::inform
//----------------------------------------------------------------------------
void
   inform(                          // Inform this Listener about
     Event&            event) const // This (application defined) Event
{  if( pub_hcdm ) debugf("Listener(%p)::inform(%p)\n", this, &event);

   function(event);
}
}; // class Listener

//----------------------------------------------------------------------------
//
// Class-
//       pub::signals::detail::ListenerList
//
// Purpose-
//       The List<Listener<Event>> container, with locking controls
//
//----------------------------------------------------------------------------
template<typename Event>
class ListenerList {                // The List of Listeners (container)
//----------------------------------------------------------------------------
// pub::signals::detail::ListenerList::Attributes
//----------------------------------------------------------------------------
protected:
typedef Listener<Event>Slot_t;      // The Listener class alias

mutable
::pub::SharedLatch     SHR;         // Protects the List of Listeners
::pub::List<Slot_t>    list;        // The actual List of Listeners

//----------------------------------------------------------------------------
// pub::signals::detail::ListenerList::Constructor
//----------------------------------------------------------------------------
public:
   ListenerList( void )             // Default constructor
:  SHR(), list()
{  if( pub_hcdm ) debugf("ListenerList(%p)::ListenerList\n", this); }

//----------------------------------------------------------------------------
// pub::signals::detail::ListenerList::Destructor
//----------------------------------------------------------------------------
   ~ListenerList( void )            // Destructor
{  if( pub_hcdm ) debugf("ListenerList(%p)::~ListenerList\n", this);

   // Implementation note: The ListenerList may contain Listeners. Only the
   // Connector may delete these objects and it can't find the ListenerList.
   // We, on the other hand, can no longer access the ListenerList because it
   // may contain deleted elements. So, no action is wanted here.
}

//----------------------------------------------------------------------------
// pub::signals::detail::ListenerList::debug
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
//       pub::signals::detail::ListenerList::inform
//
// Purpose-
//       Signal Event occurance
//
//----------------------------------------------------------------------------
void                                // (All Listeners are informed)
   inform(                          // Inform all Listeners about
     Event&            event) const // This Event
{  if( pub_hcdm ) debugf("ListenerList(%p)::inform(%p)\n", this, &event);

   std::lock_guard<decltype(SHR)> lock(SHR); // While holding the shared Latch
   for(Slot_t* slot= list.get_head(); slot; slot= slot->get_next())
     slot->inform(event);           // Inform the Listener
}

//----------------------------------------------------------------------------
// pub::signals::detail::ListenerList::insert
//----------------------------------------------------------------------------
void
   insert(                          // Insert
     Slot_t*           slot)        // This Listener (FIFO ordering)
{  if( pub_hcdm ) debugf("ListenerList(%p)::insert(%p)\n", this, slot);

   ::pub::ExclusiveLatch XCL(SHR);  // While holding the exclusive Latch
   std::lock_guard<decltype(XCL)> lock(XCL);

   list.fifo(slot);                 // Insert the Slot
}

//----------------------------------------------------------------------------
// pub::signals::detail::ListenerList::remove
//----------------------------------------------------------------------------
void
   remove(                          // Remove
     Slot_t*           slot)        // This Listener
{  if( pub_hcdm ) debugf("ListenerList(%p)::remove(%p)\n", this, slot);

   ::pub::ExclusiveLatch XCL(SHR);  // While holding the exclusive Latch
   std::lock_guard<decltype(XCL)> lock(XCL);

   list.remove(slot, slot);         // Remove the Listener
}
}; // class ListenerList
}  // namespace detail
}  // namespace pub::signals
#endif // _PUB_DETAIL_SIGNALS_H_INCLUDED
