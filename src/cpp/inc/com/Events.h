//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Events.h
//
// Purpose-
//       Event synchronization obejct.
//
// Last change date-
//       2012/01/01
//
// Notes-
//       The Events object is an inter-thread syncronization object.
//       It is useful when a thread must wait for one of several different
//       events to occur. The Events object does not indicate which of
//       these events occurred.
//
//       Only one thread, the object owner, may use the wait method.
//       Any number of threads may use the post method, any of which
//       completes a wait. As long as the number of post method calls
//       is greater than the number of wait method calls, the wait
//       method does not block.
//
// See also-
//       Status.h
//
//----------------------------------------------------------------------------
#ifndef EVENTS_H_INCLUDED
#define EVENTS_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Events
//
// Purpose-
//       Events synchronization object.
//
//----------------------------------------------------------------------------
class Events {                      // Events block
//----------------------------------------------------------------------------
// Events::Attributes
//----------------------------------------------------------------------------
private:
void*                  object;      // Hidden object

//----------------------------------------------------------------------------
// Events::Constructors
//----------------------------------------------------------------------------
public:
   ~Events( void );                 // Destructor
   Events( void );                  // Constructor

//----------------------------------------------------------------------------
// Events::Methods
//----------------------------------------------------------------------------
public:
void
   wait( void );                    // Wait for post

void
   post( void );                    // Post Events

//----------------------------------------------------------------------------
// Events::Bitwise copy prohibited
//----------------------------------------------------------------------------
private:                            // Bitwise copy is prohibited
   Events(const Events&);           // Disallowed copy constructor
   Events& operator=(const Events&);// Disallowed assignment operator
}; // class Events

#endif // EVENTS_H_INCLUDED
