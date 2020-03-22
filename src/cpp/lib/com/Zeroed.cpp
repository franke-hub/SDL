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
//       Zeroed.cpp
//
// Purpose-
//       Zeroed object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <string.h>                 // For memset

#include <com/Debug.h>

#include "com/Zeroed.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Zeroed::operator new
//
// Purpose-
//       Zeroing operator new.
//
//----------------------------------------------------------------------------
void*                               // -> Zeroed storage
   Zeroed::operator new(            // Default operator new
     size_t            size)        // Size of object
{
   void*               addr;        // Allocated address

   #ifdef HCDM
     debugf("Zeroed::operator new(%ld)\n", (long)size);
   #endif

   addr= ::operator new(size);
   memset(addr, 0, size);
   return addr;
}


void*                               // -> Zeroed storage
   Zeroed::operator new(            // In-place operator new
     size_t            size,        // Size of object
     void*             addr)        // In-place address
{
   #ifdef HCDM
     debugf("Zeroed::operator new(%ld, %p)\n", (long)size, addr);
   #endif

   memset(addr, 0, size);
   return addr;
}

//----------------------------------------------------------------------------
//
// Method-
//       Zeroed::operator delete
//
// Purpose-
//       Zeroing operator delete
//
//----------------------------------------------------------------------------
void
   Zeroed::operator delete(         // Default operator delete
     void*             addr)        // -> Object
{
   #ifdef HCDM
     debugf("Zeroed::operator delete(%p)\n", addr);
   #endif

   ::operator delete(addr);
}

void
   Zeroed::operator delete(         // In-place operator delete
     void*             addr,        // -> Object
     void*             dupAddr)     // In-place address
{
   #ifdef HCDM
     debugf("Zeroed::operator delete(%p,%p)\n", addr, dupAddr);
   #endif
}

