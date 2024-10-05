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
//       2024/10/04
//
// Implementation notes-
//       No static implementation. Thing.cpp does not exist.
//
//----------------------------------------------------------------------------
#ifndef THING_H_INCLUDED
#define THING_H_INCLUDED

#include <memory>                   // For invoker's std::make_shared

#include <pub/Object.h>             // For pub::Object, base class
#include "Counter.h"                // For Counter

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
Counter                counter;     // Constructor/destructor counter

//----------------------------------------------------------------------------
// Thing::Constructors/destructor
//----------------------------------------------------------------------------
public:
   Thing( void )                    // Constructor
:  pub::Object() {}

// Destructor declaration not required
}; // class Counter
#endif // THING_H_INCLUDED
