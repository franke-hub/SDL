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
//       Allocator.h
//
// Purpose-
//       Storage allocator description.                                                                 ts.
//
// Last change date-
//       2020/07/88
//
//----------------------------------------------------------------------------
#ifndef _PUB_ALLOCATOR_H_INCLUDED
#define _PUB_ALLOCATOR_H_INCLUDED

#include <sys/types.h>              // For size_t
#include "config.h"                 // For _PUB_NAMESPACE

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       Allocator
//
// Purpose-
//       Allocator descriptor. (Base class)
//
// Implementation notes-
//       The generic Allocator allocates variable size blocks. However,
//       Allocator subclasses may restrict usage to one fixed length
//       block size. These implementations MAY choose to ignore the
//       (defaulted) get and put length option.
//
//       The base class is implmented using malloc/free. This put method
//       implementation ignores the length parameter.
//
//----------------------------------------------------------------------------
class Allocator {                   // Allocator descriptor
//----------------------------------------------------------------------------
// Allocator::Attributes
//----------------------------------------------------------------------------
// The base class does not define attributes.

//----------------------------------------------------------------------------
// Allocator::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
public:
virtual
   ~Allocator( void ) = default;    // Default Destructor
   Allocator( void ) = default;     // Default Constructor

   Allocator(const Allocator&) = delete; // NO copy constructor
Allocator&
   operator=(const Allocator&) = delete; // NO assignment operator

//----------------------------------------------------------------------------
//
// Method-
//       check
//
// Purpose-
//       Consistency check.
//
//----------------------------------------------------------------------------
public:
virtual int                         // Return code, 0 OK
   check( void ) { return 0; }      // The base class does nothing

//----------------------------------------------------------------------------
//
// Method-
//       debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
public:
virtual void
   debug( void ) {}                 // The base class does nothing

//----------------------------------------------------------------------------
//
// Method-
//       get
//
// Purpose-
//       Allocate storage
//
// Implementation notes-
//       DOES NOT return nullptr, but may throw std::bad_alloc.
//       A fixed length (block) Allocator MAY ignore the length option.
//
//----------------------------------------------------------------------------
public:
virtual void*                       // The allocated storage (never nullptr)
   get(                             // Allocate storage
     size_t            size= 0);    // Of this length

virtual void*                       // The allocated storage (never nullptr)
   get(                             // Allocate storage
     size_t            size,        // Of this length
     size_t            align);      // And this (power of two) alignment

//----------------------------------------------------------------------------
//
// Method-
//       put
//
// Purpose-
//       Release storage.
//
// Implementation notes-
//       The size option is ignored for fixed length (block) allocators.
//
//----------------------------------------------------------------------------
virtual void
   put(                             // Deallocate
     void*             addr,        // This storage
     size_t            size= 0);    // Of this length (Optional)
}; // class Allocator
}  // namespace _PUB_NAMESPACE
#endif // _PUB_ALLOCATOR_H_INCLUDED
