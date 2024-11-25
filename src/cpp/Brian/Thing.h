//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Thing.h
//
// Purpose-
//       Debugging object (with reference Counter.)
//
// Last change date-
//       2024/11/01
//
//----------------------------------------------------------------------------
#ifndef THING_H_INCLUDED
#define THING_H_INCLUDED

#include <memory>                   // For invoker's std::make_shared

#include <pub/Object.h>             // For pub::Object, base class
#include <pub/diag-counter.h>       // For pub::diag::Counter (DEBUGGING)

//----------------------------------------------------------------------------
//
// Class-
//       Thing
//
// Purpose-
//       An Object with a reference counter.
//
//----------------------------------------------------------------------------
class Thing : public pub::Object {  // Object with reference Counter
//----------------------------------------------------------------------------
// Thing::Attributes
//----------------------------------------------------------------------------
protected:
pub::diag::Counter     counter;     // Constructor/destructor counter

//----------------------------------------------------------------------------
// Thing::Constructors/destructor
//----------------------------------------------------------------------------
public:
   Thing( void )                    // Constructor
:  pub::Object() {}

// Destructor declaration not required
}; // class Thing

//----------------------------------------------------------------------------
//
// Class-
//       NoisyThing
//
// Purpose-
//       An Object with a noisy constructor and destructor
//
//----------------------------------------------------------------------------
class NoisyThing {                  // The NoisyThing
//----------------------------------------------------------------------------
// NoisyThing::Constructors/destructor
//----------------------------------------------------------------------------
public:
   NoisyThing( void );              // Constructor

   ~NoisyThing( void );             // Destructor
}; // class NoisyThing
#endif // THING_H_INCLUDED
