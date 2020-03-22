//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Action.h
//
// Purpose-
//       Graphical User Interface: Action item
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_ACTION_H_INCLUDED
#define GUI_ACTION_H_INCLUDED

#ifndef GUI_TYPES_H_INCLUDED
#include "Types.h"                  // This include is guaranteed
#endif

#include "namespace.gui"            // Graphical User Interface
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Event;
class Object;

//----------------------------------------------------------------------------
//
// Class-
//       Action
//
// Purpose-
//       Action item, handles callback events
//
// Usage notes-
//       An Action item handles callback events for an Object.
//
//----------------------------------------------------------------------------
class Action {                      // Action item
//----------------------------------------------------------------------------
// Action::Attributes
//----------------------------------------------------------------------------
private:                            // Not modifiable by derived objects
Object*                parent;      // Parent Object
Action*                next;        // Next Action
friend class Object;

//----------------------------------------------------------------------------
// Action::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Action( void );                 // Destructor
   Action(                          // Constructor
     Object*           parent = NULL); // -> Parent Object

//----------------------------------------------------------------------------
//
// Public method-
//       Object::getParent          (Get parent)
//       Object::getNext            (Get next Action)
//
// Purpose-
//       Accessor methods.
//
//----------------------------------------------------------------------------
public:
inline Object*                      // The parent Object
   getParent( void ) const          // Get parent Object
{  return parent;
}

inline Action*                      // The next Action
   getNext( void ) const            // Get next Action
{  return next;
}

//----------------------------------------------------------------------------
//
// Public method-
//       Action::callback
//
// Purpose-
//       Handle Windows callback.
//
//----------------------------------------------------------------------------
public:
virtual void
   callback(                        // Handle callback
     const Event&      e) = 0;      // For this Event
}; // class Action
#include "namespace.end"

#endif // GUI_ACTION_H_INCLUDED
