<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023-2024 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/doc/cpp/Signals.md
//
// Purpose-
//       Signals.h reference manual
//
// Last change date-
//       2024/03/30
//
-------------------------------------------------------------------------- -->
## pub::Signals
\#include <pub/Signals.h>
### Overview
Briefly, the Signal interface consists of three objects:
- Event: The (application defined) signal Event parameter object
- Connector: (Loosely) connects a Signal and a Listener
- Signal: The signal generation object

An important internal object also exists:
- Listener: The Event handler container

The application defines the Event parameter type, the Event function handler
code, and objects it uses to generate Signals.
It uses Connector objects to manage the loose coupling between Signal and
Listener objects.
Connector::disconnect, Connector::reset, or Connector::~Connector disconnects
an association.
Signal::reset or Signal::~Signal disconnects all of that Signal's
associations.

The Listener is an internal object managed by the Signal interface which
contains the functional Event handler. That is, the Listener contains the
std::function<Event&> which defines the Event processing logic.

An application defines the Event, the parameter passed to an Event handler.
Signal processing uses this Event as a template to define the
Connector, Signal, and Listener objects.

Connectors objects cannot be copied, they can only be moved.
The Connector manages a loosely coupled Signal and Listener association.
Method Signal::connect creates a Listener and its Connector.
Upon return this Connector is moved into the application's Connector.
Methods Connector::disconnect, Connector::reset and Connector::~Connector undo
this connection and delete the Listener.

The Signal object contains a Listener list.
When an application detects an event, it invokes Signal::signal which then
serially traverses the Listener list, invokes each currently connected Event
handlers.

When an application invokes Signal::signal, signal serially invokes
each connected Event handler. All connected Event handlers will have
been called before signal returns. No thread switching occurs.

#### Static initialization considerations:

When using static Signal objects and static lambda initializers, you need to
consider initializer ordering. e.g.

----
Module 1:
```
pub::signals:Signal<const char*> my_global_signal; // (Also defined as extern)
```

Module 2:
```
pub::signals:Connector<const char*> my_global_signal_handler=
    my_global_signal.connect([](const char* info)
{   printf("Wahoo! my_global_signal_handler invoked\n"); }
```
----

If my_global_signal_handler's static initialization is invoked before
my_global_signal's construction, there's a problem.
Avoid it using the RAII (Resource Aqusition Is Initialization) algorithm:

----
Module 1:
```
static pub::signals:Signal<const char*>* the_global_signal= nullptr;

pub::signals:Signal<const char*>* my_global_signal() // (Also defined as extern)
{                                   // The (hidden) implementation
   static std::mutex mutex;
   std::lock_guard<std::mutex> lock(mutex);
   if( the_global_symbol == nullptr )
     the_global_symbol= new pub::signals:Signal<const char*>();

   return the_global_symbol;
}


namespace {
static struct cleanup {             // On termination, delete the_global_signal
   ~cleanup() { delete the_global_symbol; the_global_symbol= nullptr; }
}  my_cleanup;
}  // (Anonymous namespace)
```

Module 2:
```
pub::signals:Connector<const char*> my_global_signal_handler=
    my_global_signal()->connect([](const char* info)
{   printf("Wahoo! my_global_signal_handler invoked\n"); }
```
----

#### Usage restrictions:

Signal handlers run holding a SHR_latch on the Listener's Connector list.
Event handlers **MUST NOT** create or remove Connectors for the same Signal
since an XCL_latch would then be required.
(To avoid a livelock condition, Event handlers should not create or remove
Connectors for *any* Signal.)

#### Thread safety:

Signal objects are NOT thread-safe.
Although locking code exists, multi-threading is untested.

#### Event attributes and methods

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [attributes](./pub_signals.md#e_attributes) | Attributes (user defined) |
| [constructors](./pub_signals.md#e_constructors) | Create an Event object |


#### Connector attributes and methods

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [attributes](./pub_signals.md#c_attributes) | (Private) attributes |
| [constructors](./pub_signals.md#c_constructors) | Create a Clock object |
| [disconnect](./pub_signals.md#c_disconnect) | Disconnect (reset) this Connector. |
| [reset](./pub_signals.md#c_reset) | Reset (disconnect) this Connector. |

(The disconnect method is an alias for reset.)


#### Signal attributes and methods

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [attributes](./pub_signals.md#s_attributes) | (Private) attributes |
| [constructors](./pub_signals.md#s_constructors) | Create a Clock object |
| [connect](./pub_signals.md#s_connect) | Connect lambda function. |
| [reset](./pub_signals.md#s_reset) | Reset all connections. |
| [signal](./pub_signals.md#s_signal) | Drive all connections. |

Example-
```
   using namespace pub::signals;    // For pub::signals::Signal, Connector, ...

// Signal event definition:
struct SIG {                        // The signal event type
int                    id;          // The interrupt ID
   SIG(int id) : id(id) {}          // Constructor
};
Signal<SIG> interrupt;              // A Signal source
// :
// Drive all connected signal handlers:
signal_generator void(int id) {
   SIG sig(id);
   interrupt.signal(sig);
}
// :
// :
// Create and connect an interrupt Listener (i.e. an Event handler)
Connector<SIG> intConnector_1=
   interrupt.connect([](SIG& sig) { // (The Event handler lambda function)
     printf("Signal(%d) one\n", sig.id);
   });
// :
// Create and connect another interrupt Listener
Connector<SIG> intConnector_2=
   interrupt.connect([](SIG& sig) { // (The Event handler lambda function)
     printf("Signal(%d) two\n", sig.id);
   });
// :
// :
// Signal activation:
   signal_generator(42);            // Drives both Event handlers
```

Complex example, showing connector scope
```
   using namespace pub::signals;    // For Signal, Connector
   struct ButtonEvent {
       int ID, X, Y;                // Button id, X-Y location
       ButtonEvent(int id, int x, int y) : ID(id), X(x), Y(y) {}
   };
   struct MouseEvent {              // Mouse movement X-Y location
      int X; int Y;
       MouseEvent(int x, int y) : X(x), Y(y) {}
   };
   typedef Signal<ButtonEvent>    ButtonSignal;
   typedef Signal<MouseEvent>     MouseSignal;
   typedef Connector<ButtonEvent> ButtonConnector;
   typedef Connector<MouseEvent>  MouseConnector;

   // Handlers including lambda functions
   struct ButtonHandler {     // Defines a ButtonEvent handler
     void operator()(ButtonEvent& E) {
       printf("B1 Button X(%u) Y(%u) press id(%d)\n"
             , E.X, E.Y, E.ID);
     }
   };
   struct MouseHandler {      // Defines a MouseEvent handler
     void operator()(MouseEvent& E) {
       printf("M1  Mouse X(%u) Y(%u) moved\n", E.X, E.Y);
     }
   };

   // Screen: Contains Signals and Signal driver functions
   struct Screen {            // Holds Signals
      Signal<MouseEvent>  moved; // The MouseEvent Signal
      Signal<ButtonEvent> clicked; // The ButtonEvent Signal
      void mouse_move(int x, int y)
      {  MouseEvent E(x,y); moved.signal(E); }
      void butt_click(int id, int x, int y)
      {  ButtonEvent E(id, x, y); clicked.signal(E); }
   } S;

   // Button connector: Lambda function defined in struct
   ButtonConnector b1= S.clicked.connect(ButtonHandler());
   // Button connector: Lambda function defined here
   ButtonConnector b2= S.clicked.connect([](ButtonEvent& E) {
      printf("B2 Button X(%u) Y(%u) press id(%d)\n", E.X, E.Y, E.ID);
   });

   // Mouse connector: Lambda function defined in struct
   MouseConnector m1= S.moved.connect(MouseHandler());
   // Mouse connector: Lambda function defined here
   MouseConnector m2= S.moved.connect([](MouseEvent& E) {
      printf("M2  Mouse X(%u) Y(%u) moved\n", E.X, E.Y);
   });

   // Scope for MouseConnector m3
   {{{{                        // For m3 scoped connector
     MouseConnector m3= S.moved.connect([](MouseEvent& E) {
        printf("M3  Mouse X(%u) Y(%u) moved\n", E.X, E.Y);
     });
     S.mouse_move(12,34);      // Drives M1, M2, and M3
     S.butt_click(99, 43, 21); // Drives B1, B2
   }}}}                        // (Invokes m3.~MouseConnector())

   // m3 is now out of scope, therefore it's removed from Connector list
   printf("\nConnector reset test =============\n");
   m2.reset();                 // Disconnect MouseConnector m2
   S.clicked.reset();          // Disconnect all S.clicked connections
   b1= S.clicked.connect(ButtonHandler()); // Reconnect B1 connection
   S.mouse_move(56,78);        // Drives M1
   S.butt_click(66, 87, 65);   // Drives B1
```

Readers are encouraged to [look at](../../src/cpp/lib/pub/Test/Quick.cpp) and
run the Signal test case (from "~/obj/cpp/lib/Test")
using the "make Quick; Quick --signal --verbose" commands.
