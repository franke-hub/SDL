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
//       Xcb/Signal.h
//
// Purpose-
//       Signal descriptor.
//
// Last change date-
//       2020/10/07
//
//----------------------------------------------------------------------------
#ifndef XCB_SIGNAL_H_INCLUDED
#define XCB_SIGNAL_H_INCLUDED

#include <functional>               // For std::function
#include <memory>                   // For std::shared_ptr, std::weak_ptr
#include <mutex>                    // For std::lock_guard

#include <pub/List.h>               // For pub::List
#include <pub/Latch.h>              // For pub::ExclusiveLatch, pub::SharedLatch

#include <Xcb/Global.h>             // For opt_hcdm, opt_verbose, debugging
#include "Xcb/SignalDetail.h"       // For Signal internal classes
#include "Xcb/Widget.h"             // For xcb::Widget

namespace xcb {
//----------------------------------------------------------------------------
//
// Struct-
//       xcb::Event
//
// Purpose-
//       Event descriptor
//
// Implementation note-
//       The Signal interface does not required Events to be derived from
//       this class.
//
//----------------------------------------------------------------------------
struct Event {                      // Event descriptor
// xcb::Event::Attributes
uint8_t                type;        // Event (sub)type
uint8_t                detail[3];   // Event detail
xcb_point_t            offset;      // XY offset (May be Pixel or Column)

Widget*                widget;      // The Widget originating this Event

// xcb::Event::Constructors
   Event( void )                    // Default constructor
:  type(0), offset({0,0}), widget(nullptr)
{  detail[0]= 0; detail[1]= 0; detail[2]= 0; }

   Event(                           // Constructor
     Widget*           widget)      // The source Widget
:  type(0), offset({0,0}), widget(widget)
{  detail[0]= 0; detail[1]= 0; detail[2]= 0; }

   Event(                           // Constructor
     Widget*           widget,      // The source Widget
     int               type)        // The Event type
:  type(type), offset({0,0}), widget(widget)
{  detail[0]= 0; detail[1]= 0; detail[2]= 0; }

// xcb::Event::Destructor
virtual
   ~Event( void ) = default;        // Destructor
}; // struct Event

//----------------------------------------------------------------------------
//
// Class-
//       xcb::Connector<Event>
//
// Purpose-
//       User controlled Signal/Listener pair connection tracking.
//
//----------------------------------------------------------------------------
template<typename Event>
class Connector {                   // Signal/Listener Connector
//----------------------------------------------------------------------------
// xcb::Connector::Attributes
//----------------------------------------------------------------------------
protected:
typedef ListenerList<Event>
                       List_t;      // ListenerList type
typedef ::std::shared_ptr<List_t>
                       Parm_t;      // ListenerList<Event> (shr_pointer) type
typedef ::std::weak_ptr<List_t>
                       Weak_t;      // ListenerList<Event> (weak_pointer) type
typedef Listener<Event>*
                       Item_t;      // Listener<Event> (cpp_pointer) type

Weak_t                 list;        // The ListenerList<Event> (weak_pointer)
Item_t                 item;        // The Listener<Event> cpp_pointer

//----------------------------------------------------------------------------
// xcb::Connector::Constructors/Destructor
//----------------------------------------------------------------------------
public:
   Connector( void )                // Default constructor
:  list(), item(nullptr)
{  if( opt_hcdm ) debugf("Connector(%p.%zd)::Connector\n", this, sizeof(*this)); }

   Connector(                       // Constructor
     const Parm_t&     parm,        // The ListenerList<Event> (strong ptr)
     Item_t            item)        // The ListenerEvent (pointer)
:  list(parm), item(item)
{  if( opt_hcdm )
     debugf("Connector(%p)::Connector(%p.%zd,%p.%zd)\n", this
           , parm.get(), sizeof(List_t), item, sizeof(*item));
}

   Connector(const Connector&) = delete; // *NO* copy constructor

   Connector(
     Connector&&       that)        // MOVE constructor (resets source)
:  list(), item(nullptr)
{  if( opt_hcdm ) debugf("Connector(%p)::Connector(&&%p)\n", this, &that);

   list= ::std::move(that.list);
   item= ::std::move(that.item);
   that.list.reset();
}

   ~Connector( void )               // Destructor
{  if( opt_hcdm ) debugf("Connector(%p)::~Connector\n", this);

   reset();
}

//----------------------------------------------------------------------------
// xcb::Connector::Operators
//----------------------------------------------------------------------------
public:
Connector& operator=(const Connector&) = delete; // *NO* copy assignment

Connector&
   operator=(Connector&& that)      // MOVE assignment (resets source)
{  if( opt_hcdm ) debugf("Connector(%p)::operator=(%p)\n", this, &that);

   reset();
   list= ::std::move(that.list);
   item= ::std::move(that.item);
   that.list.reset();
   return *this;
}

//----------------------------------------------------------------------------
// xcb::Connector::debug() // Debugging display
//----------------------------------------------------------------------------
void
   debug(const char* info= nullptr)
{
   if( info == nullptr ) info= "";
   debugf("Connector(%p.%zd)::debug(%s) lock_state<%s> item(%p)\n", this
         , sizeof(*this), info, this->list.lock() ? "valid" : "gone", item);
}

//----------------------------------------------------------------------------
// xcb::Connector::reset() // Reset this Connector, removing item from list
//----------------------------------------------------------------------------
void
   reset( void )                    // Reset Connector
{  if( opt_hcdm ) debugf("Connector(%p)::reset lock_state<%s>\n", this
                        , this->list.lock() ? "valid" : "gone");

   auto list= this->list.lock();
   if( list )
     list->remove(item);

   this->list.reset();              // (Prevent duplicate remove)
   this->item= nullptr;             // Not necessary, but cleaner
}
}; // class Connector

//----------------------------------------------------------------------------
//
// Class-
//       xcb::Signal
//
// Purpose-
//       Signal descriptor
//
//----------------------------------------------------------------------------
template<typename Event>
class Signal : public ::pub::Named { // Signal<Event> descriptor
//----------------------------------------------------------------------------
// xcb::Signal::Attributes
//----------------------------------------------------------------------------
protected:
::std::shared_ptr<ListenerList<Event>>
                       list;        // The ListenerList List accessor
Widget*                owner;       // The Widget contining this Signal

//----------------------------------------------------------------------------
// xcb::Signal::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   Signal(
     Widget*           owner,       // The owning Widget
     const char*       name= nullptr) // Signal name
:  Named(name ? name : "Signal")
,  list(std::make_shared<ListenerList<Event>>())
,  owner(owner)
{
   if( opt_hcdm )
     debugf("Signal(%p)::Signal(%p,%s)\n", this, owner, get_name().c_str());
}

   ~Signal( void )
{  if( opt_hcdm ) debugf("Signal(%p)::~Signal\n", this); }

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Signal::connect
//
// Purpose-
//       Connect an Event handler to this Signal
//
// Implementation note-
//       The Connector<Event> keeps the connection active.
//
//----------------------------------------------------------------------------
Connector<Event>                    // Event handler scope Connector
   connect(                         // Connect an Event handler
     const std::function<int(const Event&)>& // The function signature
                       handler)     // The function handler
{  if( opt_hcdm )
     debugf("Signal(%p)::connect(%p.%zd)\n", this, &handler, sizeof(handler));

   Listener<Event>* item= new Listener<Event>(handler);

   list->insert(item);
   Connector<Event> c(list, item);
   return c;
}

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Signal::raise
//       xcb::Signal::inform
//
// Purpose-
//       Signal Event occurance
//
// Implemenntation notes-
//       Both raise and inform
//
//
//
//       For raise, a non-zero Listener::handle return code terminates
//       signal propagation.
//       For inform, the Listener::handle return code is ignored.
//
//----------------------------------------------------------------------------
int                                 // Return code, non-zero terminates
   raise(                           // Signal all Listeners for
     const Event&      event) const // This Event
{  if( opt_hcdm ) debugf("Signal(%p)::raise(%p)\n", this, &event);
   return list->raise(event);
}

void                                // (All Listeners are informed)
   inform(                          // Inform all Listeners about
     const Event&      event) const // This Event
{  if( opt_hcdm ) debugf("Signal(%p)::inform(%p)\n", this, &event);
   list->inform(event);
}
}; // class Signal
}  // namespace xcb
#endif // XCB_SIGNAL_H_INCLUDED
