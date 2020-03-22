//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Must.h
//
// Purpose-
//       Redefined interfaces that throw bad_alloc iff failure.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef _PUB_MUST_H_INCLUDED
#define _PUB_MUST_H_INCLUDED

#include <new>                      // For bad_alloc
#include <stdio.h>                  // For fprintf()
#include <stdlib.h>                 // For ::free, ::malloc
#include <string.h>                 // For ::strdup

#include "config.h"                 // For _PUB_NAMESPACE

namespace _PUB_NAMESPACE::Must {    // Add checking to malloc storage functions
//----------------------------------------------------------------------------
// free(void*), explicitly allowing nullptr
static inline void
   free(void*          addr)        // The storage address
{
   if( addr )
     ::free(addr);
}

static inline void*                 // The allocated storage
   malloc(size_t       size)        // The allocation length
{
   void* result= ::malloc(size);
   if( result == nullptr )          // If failure
   {
     fprintf(stderr, "malloc(%zd) failure\n", size);
     throw std::bad_alloc();
   }

   return result;
}

static inline char*                 // The duplicated string
   strdup(                          // Duplicate
     const char*       source)      // This string
{
   char* result= ::strdup(source);
   if( result == nullptr )          // If failure
   {
     fprintf(stderr, "strdup(%s) failure\n", source);
     throw std::bad_alloc();
   }

   return result;
}
}  // namespace _PUB_NAMESPACE::Must
#endif // _PUB_MUST_H_INCLUDED
