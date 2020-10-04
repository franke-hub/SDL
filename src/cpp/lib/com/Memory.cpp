//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Memory.cpp
//
// Purpose-
//       Storage allocation controller.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stdlib.h>

#include <com/Unconditional.h>

#include "com/Memory.h"

//----------------------------------------------------------------------------
//
// Method-
//       Memory::~Memory
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Memory::~Memory( void )          // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Memory::Memory
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Memory::Memory( void )           // Constructor
{
   throw "Memory::Memory";          // There is no reason to construct this
}

//----------------------------------------------------------------------------
//
// Method-
//       Memory::allocate
//
// Purpose-
//       Allocate storage.
//
//----------------------------------------------------------------------------
void*                               // -> Allocated storage
   Memory::allocate(                // Allocate storage
     unsigned          size)        // Required length
{
   return must_malloc(size);        // Allocate the storage
}

//----------------------------------------------------------------------------
//
// Method-
//       Memory::release
//
// Purpose-
//       Release allocated storage.
//
//----------------------------------------------------------------------------
void
   Memory::release(                 // Release storage
     void*             addr,        // -> Allocated storage
     unsigned          size)        // Allocated length
{
   free(addr);                      // Release the storage
   (void)size;                      // (size unused)
}

