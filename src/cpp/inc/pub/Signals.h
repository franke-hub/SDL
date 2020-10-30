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
//       Signals.h
//
// Purpose-
//       pub::signals::Signal and pub::signals::Connector descriptors.
//
// Last change date-
//       2020/10/30
//
// Usage notes-
//       The Signal interface consists of these objects:
//           Event     The application-defined Signal event parameter.
//           Connector "Connects" a Signal and an Signal Event handler.
//           Signal    The Signal generation object.
//       The application defines the Event parameter type, the Event function
//       handler code, and the objects that generate Signals. The Connector
//       object "connects" the Signal object and the Signal Event handler.
//       Connector::reset or Connector::~Connector disconnects an association.
//       Signal::reset or Signal::~Signal disconnects all of that Signal's
//       associations.
//
//       When an application invokes Signal::inform, it then serially invokes
//       each connected Event handler. All connected Event handlers will have
//       been called before inform returns. No thread switching occurs.
//
// Thread safety-
//       Signal objects are NOT thread-safe.
//       Although some locking code exists, multi-threading is untested.
//       Connector move construction and assignment are definitely unsafe.
//           For thread safety, these operations must be (but aren't) atomic.
//
// Sample usage code-
//       using namespace pub::signals; // For Signal, Connector
//       struct ButtonEvent { int ID; int X; int Y;
//           ButtonEvent(int id, int x, int y)
//               : ID(id), X(x), Y(y) {}
//       };
//       struct MouseEvent { int X; int Y;
//           MouseEvent(int x, int y) : X(x), Y(y) {}
//       };
//       typedef Signal<ButtonEvent> ButtonSignal;
//       typedef Signal<MouseEvent>  MouseSignal;
//       typedef Connector<ButtonEvent> ButtonConnector;
//       typedef Connector<MouseEvent>  MouseConnector;
//
//       struct ButtonHandler {     // Defines a ButtonEvent handler
//           void operator()(ButtonEvent& E) {
//             printf("B1 Button X(%u) Y(%u) press id(%d)\n"
//                   , E.X, E.Y, E.ID);
//           }
//       };
//       struct MouseHandler {      // Defines a MouseEvent handler
//           void operator()(MouseEvent& E) {
//               printf("M1  Mouse X(%u) Y(%u) moved\n", E.X, E.Y);
//           }
//       };
//
//       struct Screen {            // Holds Signals
//          MouseSignal  moved;     // Movement Signal
//          ButtonSignal clicked;   // Button Signal
//          void mouse_move(int x, int y)
//              { MouseEvent E(x,y); moved.inform(E); }
//          void butt_click(int id, int x, int y)
//              { ButtonEvent E(id, x, y); clicked.inform(E); }
//       } S;
//
//       ButtonConnector b1= S.clicked.connect(ButtonHandler());
//       ButtonConnector b2= S.clicked.connect([](ButtonEvent& E) {
//          printf("B2 Button X(%u) Y(%u) press id(%d)\n", E.X, E.Y, E.ID);
//       });
//
//       MouseConnector m1= S.moved.connect(MouseHandler());
//       MouseConnector m2= S.moved.connect([](MouseEvent& E) {
//          printf("M2  Mouse X(%u) Y(%u) moved\n", E.X, E.Y);
//       });
//       {{{{                        // For m3 scoped connector
//         MouseConnector m3= S.moved.connect([](MouseEvent& E) {
//            printf("M3  Mouse X(%u) Y(%u) moved\n", E.X, E.Y);
//         });
//         S.mouse_move(12,34);      // Drives M1, M2, and M3
//         S.butt_click(99, 43, 21); // Drives B1, B2
//       }}}}                        // (Invokes m3.~MouseConnector())
//
//       // m3's now out of scope, therefore it's removed from Connector list
//       printf("\nConnector reset test =============\n");
//       m2.reset();                 // (As if m2.~MouseConnector() called)
//       S.clicked.reset();          // (As if S.clicked destructor called)
//       b1= S.clicked.connect(ButtonHandler()); // (Recreate B1 connection)
//       S.mouse_move(56,78);        // Drives M1
//       S.butt_click(66, 87, 65);   // Drives B1
//
//----------------------------------------------------------------------------
#ifndef _PUB_SIGNALS_H_INCLUDED
#define _PUB_SIGNALS_H_INCLUDED

#include <functional>               // For std::function
#include <memory>                   // For std::shared_ptr, std::weak_ptr
#include <mutex>                    // For std::lock_guard

#include <pub/Debug.h>              // For pub::Debug, ...
#include <pub/List.h>               // For pub::List
#include <pub/Latch.h>              // For pub::ExclusiveLatch, SharedLatch
#include <pub/Named.h>              // For pub::Named (Base class)

//----------------------------------------------------------------------------
// Macros (temporary, undefined at end)
//----------------------------------------------------------------------------
#define debugf ::pub::debugging::debugf // Prevent ADL lookup
#define pub_hcdm ::pub::debugging::options::pub_hcdm

#include "detail/Signals.h"         // For internal classes

namespace pub::signals {
//----------------------------------------------------------------------------
//
// Typename-
//       pub::signals::Function<Event>
//
// Purpose-
//       Define the (application-implemented) Signal Event handling function.
//
//----------------------------------------------------------------------------
template <typename Event>
using Function=        std::function<void(Event&)>; // Function<Event> class

//----------------------------------------------------------------------------
//
// Class-
//       pub::signals::Connector<Event>
//
// Purpose-
//       User controlled Signal/Function<Event> pair connection tracking.
//
// Implementation note-
//       The Connector (and only the Connector) is reposible for deleting the
//       (Slot_t) Listener. That's done in method reset, either when called
//       directly by an application or by Connect::~Connect. Both the move
//       contructor and operator transfer their Listener's ownership.
//
//----------------------------------------------------------------------------
template<typename Event>
class Connector {                   // Signal/Listener Connector
//----------------------------------------------------------------------------
// pub::signals::Connector::Attributes
//----------------------------------------------------------------------------
protected:
typedef detail::ListenerList<Event>
                       List_t;      // ListenerList type
typedef detail::Listener<Event>
                       Slot_t;      // Listener type
typedef ::std::shared_ptr<List_t>
                       Strong_t;    // shared_ptr<ListenerList> type
typedef ::std::weak_ptr<List_t>
                       Weak_t;      // weak_ptr<ListenerList> type

Weak_t                 list;        // The ListenerList (weak_ptr)
Slot_t*                slot;        // The Listener (raw pointer)

//----------------------------------------------------------------------------
// pub::signals::Connector::Constructors
//----------------------------------------------------------------------------
public:
   Connector( void )                // Default constructor
:  list(), slot(nullptr)
{  if( pub_hcdm )
     debugf("Connector(%p.%zd)::Connector\n", this, sizeof(*this));
}

   Connector(                       // Constructor
     Strong_t&         _list,       // The ListenerList (shared_ptr reference)
     Slot_t*           _slot)       // The Listener (raw pointer)
:  list(_list), slot(_slot)
{  if( pub_hcdm )
     debugf("Connector(%p)::Connector(%p.%zd,%p.%zd)\n", this
           , _list.get(), sizeof(List_t), _slot, sizeof(*_slot));
}

   Connector(                       // MOVE constructor (resets source)
     Connector&&       that)
:  list(), slot(nullptr)
{  if( pub_hcdm ) debugf("Connector(%p)::Connector(&&%p)\n", this, &that);

   list= ::std::move(that.list);
   slot= ::std::move(that.slot);
   that.list.reset();               // (Prevent duplicate remove)
   that.slot= nullptr;              // (Prevent duplicate delete)
}

   Connector(const Connector&) = delete; // *NO* copy constructor

//----------------------------------------------------------------------------
// pub::signals::Connector::Destructor
//----------------------------------------------------------------------------
   ~Connector( void )               // Destructor
{  if( pub_hcdm ) debugf("Connector(%p)::~Connector\n", this);

   reset();
}

//----------------------------------------------------------------------------
// pub::signals::Connector::Operators
//----------------------------------------------------------------------------
Connector&
   operator=(Connector&& that)      // MOVE assignment (resets source)
{  if( pub_hcdm ) debugf("Connector(%p)::operator=(&&%p)\n", this, &that);

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
// pub::signals::Connector::debug() // Debugging display
//----------------------------------------------------------------------------
void
   debug(const char* info= nullptr)
{
   if( info == nullptr ) info= "";
   debugf("Connector(%p.%zd)::debug(%s) lock_state<%s> slot(%p)\n", this
         , sizeof(*this), info, list.lock() ? "valid" : "gone", slot);
}

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
   reset( void )                    // Reset Connector
{  if( pub_hcdm )
     debugf("Connector(%p)::reset lock_state<%s>\n", this
           , list.lock() ? "valid" : "gone");

   auto locked_list= list.lock();
   if( locked_list )
     locked_list->remove(slot);

   delete slot;                     // Delete the slot
   list.reset();                    // (Prevent duplicate remove)
   slot= nullptr;                   // (Prevent duplicate delete)
}
}; // class pub::signals::Connector

//----------------------------------------------------------------------------
//
// Class-
//       pub::signals::Signal<Event>
//
// Purpose-
//       Signal descriptor
//
//----------------------------------------------------------------------------
template<typename Event>
class Signal : public Named {       // Signal descriptor
//----------------------------------------------------------------------------
// pub::signals::Signal::Attributes
//----------------------------------------------------------------------------
protected:
typedef std::function<void(Event&)>
                       Function;   // Function<Event> class
typedef detail::Listener<Event>
                       Listener;    // Using detail::Listener
typedef detail::ListenerList<Event>
                       ListenerList; // Using detail::ListenerList

::std::shared_ptr<ListenerList>
                       list;        // The ListenerList List accessor

//----------------------------------------------------------------------------
// pub::signals::Signal::Constructors
//----------------------------------------------------------------------------
public:
   Signal(                          // Constructor
     const char*       name= nullptr) // Signal name
:  Named(name ? name : "Signal")
,  list(std::make_shared<ListenerList>())
{  if( pub_hcdm ) debugf("Signal(%p)::Signal(%s)\n", this, get_name().c_str());
}

   Signal(const Signal&) = delete;  // *NO* copy constructor

//----------------------------------------------------------------------------
// pub::signals::Signal::Destructor
//----------------------------------------------------------------------------
virtual
   ~Signal( void )                  // Destructor
{  if( pub_hcdm ) debugf("Signal(%p)::~Signal\n", this); }

//----------------------------------------------------------------------------
// pub::signals::Signal::Operators
//----------------------------------------------------------------------------
Signal& operator=(const Signal&) = delete; // *NO* Assignment operator

//----------------------------------------------------------------------------
// pub::signals::Signal::debug, Debugging display
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info= "")    // Associated text
{  debugf("Signal(%p)::debug(%s) Named(%s)\n", this, info, get_name().c_str());
   list->debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Signal::connect
//
// Purpose-
//       Connect a Signal to a (Function<Event> container) Listener
//
// Implementation note-
//       A Connector connects a Signal and a Function. Delete or reset the
//       Connector to disconnect them.
//
//----------------------------------------------------------------------------
Connector<Event>                    // The Signal/Function connector
   connect(                         // Connect a Signal Event handler
     const Function&   function)    // The Signal Event handler function
{  if( pub_hcdm )
     debugf("Signal(%p)::connect(%p.%zd)\n", this, &function, sizeof(function));

   Listener* slot= new Listener(function);

   list->insert(slot);
   Connector<Event> c(list, slot);
   return c;
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Signal::inform
//
// Purpose-
//       Inform all Listeners about an Event
//
//----------------------------------------------------------------------------
void
   inform(                          // Inform all Listeners about
     Event&            event) const // This Event
{  if( pub_hcdm ) debugf("Signal(%p)::raise(%p)\n", this, &event);

   list->inform(event);
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::signals::Signal::reset
//
// Purpose-
//       Remove all Listeners
//
//----------------------------------------------------------------------------
void
   reset( void )                    // Remove all Listeners
{  if( pub_hcdm ) debugf("Signal(%p)::reset\n", this);

   // Note: Replacing the ListenerList prevents Connectors from finding it.
   // This is required in order to prevent dangling Listener references.
   list= std::make_shared<ListenerList>(); // Replace the ListenerList
}
}; // class pub::signals::Signal
}  // namespace pub::signals
//----------------------------------------------------------------------------
// Remove temporary macros
//----------------------------------------------------------------------------
#undef debugf
#undef pub_hcdm
#endif // _PUB_SIGNALS_H_INCLUDED
