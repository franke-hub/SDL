//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2021 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DispatchDone.h
//
// Purpose-
//       Standard Dispatch Done callback objects.
//
// Last change date-
//       2021/07/09
//
//----------------------------------------------------------------------------
#ifndef _PUB_DISPATCHDONE_H_INCLUDED
#define _PUB_DISPATCHDONE_H_INCLUDED

#include <pub/Event.h>              // For Wait event

namespace _PUB_NAMESPACE::dispatch {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Item;

//----------------------------------------------------------------------------
//
// Class-
//       dispatch::Done
//
// Purpose-
//       The Dispatcher Done callback Object
//
//----------------------------------------------------------------------------
class Done {                        // The dispatch::Done callback Object
//----------------------------------------------------------------------------
// Done::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Done( void ) {}                 // Destructor
   Done( void ) {}                  // Constructor

   Done(const Done&) = delete;      // Disallowed copy constructor
   Done& operator=(const Done&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Done::Methods
//----------------------------------------------------------------------------
public:
virtual void                        // OVERRIDE this method
   done(                            // Complete
     Item*             item) = 0;   // This work Item
}; // class Done

//----------------------------------------------------------------------------
//
// Class-
//       Wait
//
// Purpose-
//       The Wait until done Object
//
// Notes-
//       This Object cannot cannot be shared, but can be reused by calling the
//       reset method once the wait has been satisfied.
//
//----------------------------------------------------------------------------
class Wait : public Done {          // The dispatcher Wait until Done Object
//----------------------------------------------------------------------------
// Wait::Attributes
//----------------------------------------------------------------------------
private:
Event                  event;       // For wait/post

//----------------------------------------------------------------------------
// Wait::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Wait( void ) {}                 // Destructor
   Wait( void ) : Done() {}         // Constructor

   Wait(const Wait&) = delete;      // Disallowed copy constructor
   Wait& operator=(const Wait&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Wait::Methods
//----------------------------------------------------------------------------
public:
virtual void
   done(                            // Complete
     Item*             )            // This (ignored) work Item
{  event.post(); }

void
   reset( void )                    // Reset for re-use
{  event.reset(); }

void
   wait( void )                     // Wait for work Item completion
{  event.wait(); }
}; // class Wait
}  // namespace _PUB_NAMESPACE::dispatch
#endif // _PUB_DISPATCHDONE_H_INCLUDED
