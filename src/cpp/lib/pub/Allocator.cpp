//----------------------------------------------------------------------------
//
//       Copyright (c) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Allocator.cpp
//
// Purpose-
//       Allocator method implementations.
//
// Last change date-
//       2020/07/08
//
//----------------------------------------------------------------------------
#include <exception>                // For std::bad_alloc
#include <stdlib.h>                 // For malloc, free

#include "pub/Allocator.h"

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Method-
//       Allocator::get
//
// Purpose-
//       Allocate storage
//
// Implementation notes-
//       DOES NOT return nullptr, but may throw std::bad_alloc.
//       A fixed length (block) Allocator MAY ignore the length option.
//
//----------------------------------------------------------------------------
void*                               // The allocated storage (never nullptr)
   Allocator::get(                  // Allocate storage
     size_t            size)        // Of this length
{
   void* result= malloc(size);      // Allocate storage
   if( result == nullptr )          // Exception if failure
     throw std::bad_alloc();

   return result;
}

void*                               // The allocated storage (never nullptr)
   Allocator::get(                  // Allocate aligned storage
     size_t            size,        // Of this length
     size_t            align)       // And this alignment
{
   void* result= aligned_alloc(size, align); // Allocate storage
   if( result == nullptr )          // Exception if failure
     throw std::bad_alloc();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::put
//
// Purpose-
//       Release storage.
//
//----------------------------------------------------------------------------
void
   Allocator::put(                  // Deallocate
     void*             addr,        // This storage
     size_t            size)        // Of this length
{  free(addr); }                    // Release storage, ignoring size
}  // namespace _PUB_NAMESPACE
