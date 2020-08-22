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
//       2020/08/21
//
//----------------------------------------------------------------------------
#ifndef _PUB_ALLOCATOR_H_INCLUDED
#define _PUB_ALLOCATOR_H_INCLUDED

#include <sys/types.h>              // For size_t

#include <pub/Latch.h>              // For Latch
#include <pub/List.h>               // For List

namespace pub {
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

//----------------------------------------------------------------------------
//
// Method-
//       put
//
// Purpose-
//       Release storage.
//
// Implementation notes-
//       A fixed length (block) Allocator MAY ignore the length option.
//
//----------------------------------------------------------------------------
virtual void
   put(                             // Deallocate
     void*             addr,        // This storage
     size_t            size= 0);    // Of this length (Optional)
}; // class Allocator

//----------------------------------------------------------------------------
//
// Class-
//       BlockAllocator
//
// Purpose-
//       Fixed size block Allocator.
//
// Implementation notes-
//       The BlockAllocator is thread-safe.
//
//       The BlockAllocator is a Pool allocator. When a BlockAllocator is
//       deleted, so is all of its associated storage. An implementation
//       MAY, but is not required to verify that all blocks have been
//       released.
//
//       The BlockAllocator allocates from larger block groups.
//       The standard Allocator is used if size differs from the block size.
//
//----------------------------------------------------------------------------
class BlockAllocator : public Allocator { // BlockAllocator descriptor
//----------------------------------------------------------------------------
// BlockAllocator::Attributes
//----------------------------------------------------------------------------
public:
enum { DIM= 4 };                    // The number of fast slots
struct Block : public SHSL_List<Block>::Link { // An allocation block
};

protected:
size_t                 b_size;      // The allocation block size
SHSL_List<Block>       b_list;      // The block list
size_t                 size;        // The allocation size

Latch                  mutex;       // Free slot allocation is single-threaded
::std::atomic<void*>   fast[DIM];   // Fast allocation/release slots
::std::atomic<void*>   free;        // Free slot chain, (A.K.A. slow slots)

//----------------------------------------------------------------------------
// BlockAllocator::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
public:
virtual
   ~BlockAllocator( void );         // Destructor
   BlockAllocator(                  // Constructor
     size_t            size,        // The allocation item size
     size_t            b_size= 0);  // The allocation block size

   BlockAllocator(const BlockAllocator&) = delete; // NO copy constructor
BlockAllocator&
   operator=(const BlockAllocator&) = delete; // NO assignment operator

//----------------------------------------------------------------------------
//
// Method-
//       BlockAllocator::get
//
// Purpose-
//       Allocate storage
//
//----------------------------------------------------------------------------
public:
virtual void*                       // The allocated storage (never nullptr)
   get(                             // Allocate storage
     size_t            size= 0);    // Of this length

//----------------------------------------------------------------------------
//
// Method-
//       BlockAllocator::put
//
// Purpose-
//       Release storage.
//
//----------------------------------------------------------------------------
virtual void
   put(                             // Deallocate
     void*             addr,        // This storage
     size_t            size= 0);    // Of this length (Optional)
}; // class BlockAllocator
}  // namespace pub
#endif // _PUB_ALLOCATOR_H_INCLUDED
