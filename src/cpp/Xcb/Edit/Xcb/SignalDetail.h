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
//       Xcb/SignalDetail.h
//
// Purpose-
//       Signal detail, not visible
//
// Last change date-
//       2020/09/06
//
// Implementation notes-
//       Only included from Xcb/Signal.h, which loads prerequisite includes
//
//----------------------------------------------------------------------------
#ifndef XCB_SIGNALDETAIL_H_INCLUDED
#define XCB_SIGNALDETAIL_H_INCLUDED

namespace xcb {
//----------------------------------------------------------------------------
//
// Class-
//       xcb::Listener<Event>
//
// Purpose-
//       Listener descriptor
//
// Implementation notes-
//       For conditional Events, a non-zero return code terminates propagation.
//       For informative Events, the return code is ignored.
//
//----------------------------------------------------------------------------
template<typename Event>
class Listener : public pub::List<Listener<Event>>::Link { // Listener descriptor
//----------------------------------------------------------------------------
// xcb::Listener::Attribute
//----------------------------------------------------------------------------
protected:
// ===========================================================================
struct Container {
std::function<int(const Event&)>
                       raised;      // The Signal handler

enum { HCDM= false };               // TRUE enables con/destructor debugging

   Container(const std::function<int(const Event&)>&source)
:  raised(source)
{  if( HCDM && opt_hcdm )
     debugf("Container(%p.%zd)::Container\n", this, sizeof(Container));
}

   ~Container( void )
{  if( HCDM && opt_hcdm ) debugf("Container(%p)::~Container\n", this); }
// ===========================================================================
}                      container; // (Contains the (copied) Signal handler)

//----------------------------------------------------------------------------
// xcb::Listener::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   Listener(                        // Constructor
     const std::function<int(const Event&)>&
                       raised)      // The Signal handler
:  container(raised)
{  if( opt_hcdm )
     debugf("Listener(%p.%zd)::Listener(%p.%zd)\n"
           , this, sizeof(*this), &raised, sizeof(raised));
}

   ~Listener( void )                // Destructor
{  if( opt_hcdm )
     debugf("Listener(%p)::~Listener\n", this);
}

//----------------------------------------------------------------------------
// xcb::Listener::raise
//----------------------------------------------------------------------------
int
   raise(                           // Handle
     const Event&      event) const // This Event
{  if( opt_hcdm )
     debugf("Listener(%p)::raise(%p)\n", this, &event);
   return container.raised(event);
}
}; // class Listener<Event>

//----------------------------------------------------------------------------
//
// Class-
//       xcb::ListenerList<Event>
//
// Purpose-
//       The Listener List container, with locking controls
//
// Implementation notes-
//       We need to have both std::strong_ptr and std::weak_ptr access to
//       the list. We can't just use the Signal object because we don't
//       control how that object is allocated. It's often on the stack, so
//       releasing it via std::shared_ptr isn't a good idea.
//
// Implementation notes-
//       The ListenerList temporarily remains even after the Signal is deleted.
//       The destructor removes any remaining Listeners from the list (when
//       no other shared_ptr references to it remain.)
//
//----------------------------------------------------------------------------
template<typename Event>
class ListenerList {                // The List of Listeners
//----------------------------------------------------------------------------
// xcb::ListenerList::Attributes
//----------------------------------------------------------------------------
protected:
typedef Listener<Event>*
                       Listener_t;  // The Listener (pointer) type

mutable
::pub::SharedLatch     mutex;       // Protects listeners

::std::shared_ptr<::pub::List<Listener<Event>>>
                       list;        // The actual List of Listeners

//----------------------------------------------------------------------------
// xcb::ListenerList::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   ListenerList( void )             // Default constructor
:  mutex(),  list(std::make_shared<::pub::List<Listener<Event>>>())
{  if( opt_hcdm ) debugf("ListenList(%p)::ListenList\n", this); }

   ~ListenerList( void )            // Destructor
{  if( opt_hcdm ) debugf("ListenList(%p)::~ListenList\n", this);
// No more references to the ListenerList exist, so locking isn't needed.
// In particular, Connectors can no longer find this list, so they will not
// be able to remove their associated Listener. It must be done here.

// pub::ExclusiveLatch mutex(this->mutex);
// std::lock_guard<decltype(mutex)> lock(mutex); // Holding the exclusive Latch

   Listener_t L= list->remq();      // Delete all left over Listeners
   while( L ) {
     delete L;
     L= list->remq();
   }
}

//----------------------------------------------------------------------------
// xcb::ListenerList::insert, fifo, lifo
//----------------------------------------------------------------------------
void
   insert(                          // Insert
     Listener_t        listener)    // This Listener (FIFO ordering)
{  if( opt_hcdm ) debugf("ListenList(%p)::insert(%p)\n", this, listener);

   pub::ExclusiveLatch mutex(this->mutex); // Holding the exclusive Latch
   std::lock_guard<decltype(mutex)> lock(mutex);

   list->fifo(listener);            // Insert the Listener
}

//----------------------------------------------------------------------------
// xcb::ListenerList::remove
//----------------------------------------------------------------------------
void
   remove(                          // Remove (and delete)
     Listener_t        listener)    // This Listener
{  if( opt_hcdm ) debugf("ListenList(%p)::remove(%p)\n", this, listener);

   pub::ExclusiveLatch mutex(this->mutex); // Holding the exclusive Latch
   std::lock_guard<decltype(mutex)> lock(mutex);

   list->remove(listener, listener); // Remove the Listener
   delete listener;                 // And delete it
}

//----------------------------------------------------------------------------
//
// Method-
//       xcb::ListenerList::raise
//       xcb::ListenerList::inform
//
// Purpose-
//       Signal Event occurance
//
// Implemenntation notes-
//       For raise, a non-zero Listener::handle return code terminates
//       signal propagation.
//       For inform, the Listener::handle return code is ignored.
//
//----------------------------------------------------------------------------
int                                 // Return code, non-zero terminates
   raise(                           // Signal all Listeners for
     const Event&      event) const // This Event
{  if( opt_hcdm ) debugf("ListenList(%p)::raise(%p)\n", this, &event);

   std::lock_guard<decltype(mutex)> lock(mutex); // Holding shared Latch

   for(Listener_t L= list->get_head(); L; L= L->get_next()) {
     int rc= L->raise(event);       // Invoke the Listener, honoring return
     if( rc )
       return rc;
   }

   return 0;
}

void                                // (All Listeners are informed)
   inform(                          // Inform all Listeners about
     const Event&      event) const // This Event
{  if( opt_hcdm ) debugf("ListenList(%p)::inform(%p)\n", this, &event);

   std::lock_guard<decltype(mutex)> lock(mutex); // Holding shared Latch

   for(Listener_t L= list->get_head(); L; L= L->get_next()) {
     L->raise(event);               // Invoke the Listener, ignoring return
   }
}
}; // class ListenerList
}  // namespace xcb
#endif // XCB_SIGNALDETAIL_H_INCLUDED
