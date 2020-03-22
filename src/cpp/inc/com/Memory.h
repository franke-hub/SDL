//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Memory.h
//
// Purpose-
//       Storage allocation controller.
//
// Last change date-
//       2007/10/10
//
//----------------------------------------------------------------------------
#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Memory
//
// Purpose-
//       Storage allocation controller.
//
//----------------------------------------------------------------------------
class Memory {                      // Storage allocation controller
//----------------------------------------------------------------------------
// Memory::Constructors/Destructors
//----------------------------------------------------------------------------
public:
   ~Memory( void );                 // Destructor
   Memory( void );                  // Default Constructor

//----------------------------------------------------------------------------
// Memory::Methods
//----------------------------------------------------------------------------
public:
static void*                        // -> Allocated storage (Exception on failure)
   allocate(                        // Allocate storage
     unsigned          size);       // Required length

static void
   release(                         // Release storage
     void*             addr,        // Allocated address
     unsigned          size);       // Allocated length
}; // class Memory

#endif // MEMORY_H_INCLUDED
