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
//       ~/doc/cpp/pub_signals.md
//
// Purpose-
//       Signals.h reference manual
//
// Last change date-
//       2024/01/23
//
-------------------------------------------------------------------------- -->
## Defined in header <pub/Signals.h>

### pub\::signals::

## Signals
Signals objects are defined using namespace pub::signals.
All objects use template<Event>, where Event is a user-defined struct.

Attribute declarations in this manual may not exactly match the implementation.
Typedefs are used to simplify the code.

<!-- ===================================================================== -->
---
### Event
The Event is a user-defined struct or class containing parameters passed
to the Listener, i.e. the Event handler.
An Event is *completely* application defined.
It may contain attributes and methods unrelated to Signal processing.


#### <a id="e_attributes">Attributes:</a>
- Event attributes are user-defined
- The Event object is passed by reference to each Signal listener.

For the simple example of a mouse button press Event handler, the Event might
contain the button identifier and the mouse X/Y location, e.g.

```
   struct ButtonEvent {             // (A Button pressed Event)
       int ID, X, Y;                // Button id, X/Y location
       ButtonEvent(int id, int x, int y) : ID(id), X(x), Y(y) {}
   };
```

#### <a id="e_constructors">Constructor:</a>
While the example (ButtonEvent) Event contains a constructor, constructors
are not necessary.

Since the Event will be passed by reference to Listener methods,
they can freely modify it.
This allows application flexibility.

The current Signal implementation drives current Listeners in the order that
their Signal::connect calls were processed.
However, applications are somewhat discouraged from relying on this order.
Applications change over time and, while there is no current reason why this
might be needed, it *is* possible that the Signal implementation might change.


<!-- ===================================================================== -->
---
### Connector<Event>
The Connector manages the loose Signal/Listener affiliation.

#### <a id="c_attributes">Attributes</a>
Attribute information is provided to help in understanding Connector logic.

protected:
- std::weak_ptr<ListenerList<Event>> list; // A weak_ptr reference to the
associated Signal's list of Listeners.
- Listener<Event>* slot; // A raw pointer to the Listener. When active,
Connectors *own* the Listener object. (When inactive, it contains nullptr.)

#### <a id="c_constructors">Constructors</a>

public:
- Connector( void ); // The default constructor sets both the list and the
slot to nullptr.
- Connector(
std::shared_ptr<ListenerList<Event>>& strong, /* A strong reference to the
Signal's ListenerList */
Listener<Event>* raw /* A pointer to the Listener */
); // Signal::connect invokes this constructor to fully initialize it.
(The constructor saves a weak_ptr<ListenerList<Event>>)
- Connector(const Connector&) =  delete; // *NO* copy constructor
- Connector(Connector&& that); // ONLY the move constructor is provided

#### <a id="c_operators">Operators</a>

- operator=(const Connector&) =  delete; // *NO* copy assignment
- operator=(Connector&& that); // ONLY move assignment is provided

Since a Connector *owns* the associated Listener and must eventually delete
it, no copy constructor or assignment operator is logically possible.
Only the move constructor and assignment operators are provided.

#### Methods

- <a id="c_disconnect">void disconnect(void) { reset(); }</a> // disconnect is
an alias for reset. Use whichever method name you prefer
- <a id="c_reset">void reset(void);</a> // Reset the Connector, restoring
its default state. The Listener is removed from the Signal's ListenerList
and deleted.


<!-- ===================================================================== -->
---
### Signal<Event>
The Signal object contains the list of associated Listeners and notifies
these Listeners upon request.

Signal provides the interface that *adds* Listener elements to the ListenerList
but never removes them.
Connector objects are associated with each Listener.
Only the Connector removes Listener elements from the Signal's ListenerList.

However, a Signal can be reset.
Reset replaces the ListenerList with a new ListenerList, effectively removing
all active Connections.


#### <a id="s_attributes">Attributes</a>
Attribute information is provided to help in understanding Signal logic.

protected:
- std::shared_ptr<ListenerList<Event>> list; // The list of Listeners
associated with this Signal.

#### <a id="s_constructors">Constructors</a>

public:
- Signal( void ); // The default constructor creates a new, empty list.

#### <a id="s_operators">Operators</a>

(No operators are defined.)

#### Methods

- <a id="s_connect">Connector<Event> connect(const std::function<void(Event&)>
f)</a>;
// Creates a Listener (the function container) and inserts it onto the
ListenerList.
Method connect returns a Connector<Event> which is used by the application to
manage the connection.
- <a id="s_reset">void reset(void)</a>; // Replaces the ListenerList.
This removes all active Signal/Listener associations for this Signal.
- <a id="s_signal">void signal(Event& event)</a>; // Serially transverses the
ListenerList, invoking each active Listener.


<!-- ===================================================================== -->
---
### Listener<Event>
The Listener is an internal object, not part of the external interface.

The Listener contains a std::function<void(Event&)> function.


#### <a id="l_attributes">Attributes</a>
Attribute information is provided to help in understanding Signal logic.

protected:
- const std::function<void(Event&)> function; // The Event handler
implementation, provided by the application.

#### <a id="l_constructors">Constructors</a>

public:
- Listener(const std::function<void(Event&)> function; // The event handler
implementation, as provided to Signal::connect by the application

#### <a id="l_operators">Operators</a>

(No operators are defined.)

#### Methods

- <a id="l_signal">void signal(Event& event) const { function(event); }</a>
// (As shown) this just invokes the Event handler

---
