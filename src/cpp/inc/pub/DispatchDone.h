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
//       2021/11/09
//
//----------------------------------------------------------------------------
#ifndef _PUB_DISPATCHDONE_H_INCLUDED
#define _PUB_DISPATCHDONE_H_INCLUDED

#include <functional>               // For std::function

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
   Done( void ) {}                  // Default constructor

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
//       dispatch::LambdaDone
//
// Purpose-
//       The Dispatcher Done callback Object
//
//----------------------------------------------------------------------------
class LambdaDone : public Done {    // The dispatch::LambdaDone callback Object
//----------------------------------------------------------------------------
// LambdaDone::Attributes
//----------------------------------------------------------------------------
public:
typedef std::function<void(Item*)> function_t; // Work handler

protected:
function_t             callback;    // The Work item handler

//----------------------------------------------------------------------------
// LambdaDone::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~LambdaDone( void ) = default;   // Destructor
   LambdaDone( void )               // Default constructor
:  Done() { }                       // (Callback not initialized)
   LambdaDone(function_t f)         // Constructor
:  Done(), callback(f) {}

   LambdaDone(const LambdaDone&) = delete; // Disallowed copy constructor
   LambdaDone& operator=(const LambdaDone&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// LambdaDone::Methods
//----------------------------------------------------------------------------
public:
void
   on_done(function_t f)            // Replace callback
{  callback= f; }

virtual void
   done(                            // Complete
     Item*             item)        // This work Item
{  callback(item); }
}; // class LambdaDone

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
