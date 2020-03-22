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
//       Redefined interfaces that throw runtime_error iff failure.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef MUST_H_INCLUDED
#define MUST_H_INCLUDED

#include <stdexcept>

//----------------------------------------------------------------------------
//
// Class-
//       Must
//
// Purpose-
//       Redefine interfaces to throw exception on failure or to gracefully
//       handle normal conditions.
//
//----------------------------------------------------------------------------
class Must {                        // Throw exception on failure
public:
   Must( void ) = delete;           // No constructor
   ~Must( void ) = delete;          // No destructor

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
     throw std::runtime_error("no storage");
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
     throw std::runtime_error("no storage");
   }

   return result;
}
}; // class Must

#endif // UNCONDITIONAL_H_INCLUDED
