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
//       SubAllocator.h
//
// Purpose-
//       Storage SubAllocator description.                                                                 ts.
//
// Last change date-
//       2020/06/18
//
// Implementation note-
//       Placeholder until implemented.
//
//----------------------------------------------------------------------------
#ifndef _PUB_SUBALLOCATOR_H_INCLUDED
#define _PUB_SUBALLOCATOR_H_INCLUDED

#include <sys/types.h>              // For size_t

#include "pub/Allocator.h"          // For Allocator (base class)

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       SubAllocator
//
// Purpose-
//       SubAllocator descriptor.
//
//----------------------------------------------------------------------------
class SubAllocator : public Allocator { // SubAllocator descriptor
//----------------------------------------------------------------------------
// SubAllocator::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum
{ MAX_SIZE= size_t(0x0000000100000000) // Maximum storage size
, MIN_SIZE= size_t(0x0000000000010000) // Minimum storage size
};

//----------------------------------------------------------------------------
// SubAllocator::Attributes
//----------------------------------------------------------------------------
protected:
// To be determined

//----------------------------------------------------------------------------
// SubAllocator::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
public:
virtual
   ~SubAllocator( void ) = default; // Default Destructor
   SubAllocator(                    // Constructor
     char*             addr,        // Origin address
     size_t            size)        // Size, range 64K .. 4G
:  Allocator() {} // NOT CODED YET. Parameters ignored

   SubAllocator(const SubAllocator&) = delete; // NO copy constructor
SubAllocator&
   operator=(const SubAllocator&) = delete; // NO assignment operator

//----------------------------------------------------------------------------
//
// Method-
//       check
//
// Purpose-
//       Consistency check
//
//----------------------------------------------------------------------------
public:
// virtual int                      // Return code, 0 OK
//    check( void );                // Consistency check

//----------------------------------------------------------------------------
//
// Method-
//       debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
public:
// virtual void
//    debug( void );                // Debugging display

//----------------------------------------------------------------------------
//
// Method-
//       get
//
// Purpose-
//       Allocate storage
//
//----------------------------------------------------------------------------
public:
// virtual void*                    // The allocated storage (never nullptr)
//    get(                          // Allocate storage
//      size_t            size= 0); // Of this length
//
// virtual void*                    // The allocated storage (never nullptr)
//    get(                          // Allocate aligned storage
//      size_t            size,     // Of this length
//      size_t            align);   // And this alignment

//----------------------------------------------------------------------------
//
// Method-
//       put
//
// Purpose-
//       Release storage.
//
//----------------------------------------------------------------------------
// virtual void
//    put(                          // Deallocate
//      void*             addr,     // This storage
//      size_t            size= 0); // Of this length (Optional)
}; // class SubAllocator
}  // namespace _PUB_NAMESPACE
#endif // _PUB_SUBALLOCATOR_H_INCLUDED
