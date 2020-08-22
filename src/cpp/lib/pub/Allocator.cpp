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
//       2020/08/21
//
//----------------------------------------------------------------------------
#include <exception>                // For std::bad_alloc, ...
#include <mutex>                    // For std::lock_guard
#include <stdlib.h>                 // For malloc, free

#include <pub/Debug.h>              // For Debug, debugging
#include "pub/Allocator.h"          // Implementation interfaces

using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compile-time constants
{  ALIGN= sizeof(void*)             // Allocation alignment
,  BLOCK_ALIGN= 4096                // Block alignment
,  MALLOC_OVERHEAD= 16              // (Heuristic) malloc overhead
,  MIN_ELEMENTS= 16                 // Block minimum element count
,  OVERHEAD= sizeof(pub::BlockAllocator::Block) + MALLOC_OVERHEAD
,  USE_DELETE_VERIFY= true          // Verify all storage released?
}; // Compile-time constants

//----------------------------------------------------------------------------
// struct Free: Element on free list
//----------------------------------------------------------------------------
struct Free {
Free*                  next;
};

//----------------------------------------------------------------------------
// unexpected: Handle unexpected event
//----------------------------------------------------------------------------
[[ noreturn ]] static void
   unexpected(                      // Handle should-not-occur situation
     int               line,        // Source line number
     const char*       mess)        // Message text
{
   static char buffer[128];
   size_t L= snprintf(buffer, sizeof(buffer), "%4d Allocator: Should Not Occur: %s"
                     , line, mess);
   if( L >= sizeof(buffer) )
     L= sizeof(buffer) - 1;
   buffer[L]= '\0';

   throw std::runtime_error(buffer);
}

namespace pub {
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

//----------------------------------------------------------------------------
//
// Method-
//       BlockAllocator::BlockAllocator
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   BlockAllocator::BlockAllocator(  // Constructor
     size_t            size,        // Element size
     size_t            b_size)      // Block size (recommended)
:  Allocator(), b_list()
{
   size += (ALIGN - 1);             // Round up to alignment
   size &= ~(ALIGN - 1);
   if( size == 0 ) throw std::invalid_argument("size");
   this->size= size;

   //-------------------------------------------------------------------------
   // The specified block size must allow for at least MIN_ELEMENTS
   b_size += (ALIGN - 1);           // Round up to alignment
   b_size &= ~(ALIGN - 1);
   if( b_size < (OVERHEAD + size * MIN_ELEMENTS) )
     b_size= OVERHEAD + size * MIN_ELEMENTS;
   b_size += (BLOCK_ALIGN - 1);
   b_size &= ~(BLOCK_ALIGN - 1);
   b_size -= MALLOC_OVERHEAD;
   this->b_size= b_size;

   // Initialize the fast array and the free list
   for(unsigned i= 0; i<DIM; i++)
     fast[i].store(nullptr);
   free.store(nullptr);

   // (Storage is not allocated here in the constructor, but upon first need.)
}

//----------------------------------------------------------------------------
//
// Method-
//       BlockAllocator::~BlockAllocator
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   BlockAllocator::~BlockAllocator( void ) // Destructor
{
   // Here the latch ALSO protects the static buffer in unexpected()
   std::lock_guard<decltype(mutex)> lock(mutex);

   if( USE_DELETE_VERIFY ) {        // Verify all storage released?
     void* const DEADBEEF= (void*)uintptr_t(0xdeadbeefdeadbeef);
     size_t found= 0;               // Number of blocks found

     // Make the fast array unusable, counting elements
     for(unsigned i= 0; i<DIM; i++) {
       void* old= fast[i].load();
       while( old ) {
         if( old == DEADBEEF ) unexpected(__LINE__, "?duplicate delete?");
         if( fast[i].compare_exchange_strong(old, DEADBEEF) ) {
           found++;
           break;
         }
       }
     }

     // Make the free list unusable
     void* old= free.load();
     while( old ) {
       if( old == DEADBEEF ) unexpected(__LINE__, "?duplicate delete?");
       if( free.compare_exchange_strong(old, DEADBEEF) )
         break;
     }

     // Count the elements found on the free list
     size_t per_block= (b_size - sizeof(Block)) / size; // Units per block
     size_t total= 0;              // The number of allocated blocks
     for(Block* block= b_list.getHead(); block; block= block->getNext() )
       total += per_block;

     Free* free= (Free*)old;
     while( free ) {
       found++;
       if( found > total ) break;
       free= free->next;
     }
     //-----------------------------------------------------------------------
     // Too many free blocks indicates that a block was allocated by one
     // BlockAllocator and released to a different one.
     if( found > total ) unexpected(__LINE__, "Error: too many free blocks");
     if( found < total ) unexpected(__LINE__, "User error: memory leak");
   }

   //-------------------------------------------------------------------------
   // Free all blocks
   for(Block* block= b_list.remq(); block; block= b_list.remq())
     ::free(block);
}

//----------------------------------------------------------------------------
//
// Method-
//       BlockAllocator::get
//
// Purpose-
//       Allocate storage
//
// Implementation note-
//       Allocation from the free list requires a latch for mutual exclusion
//       between allocation requests. It requires compare_exchange logic for
//       concurrency with (lock free) release requests.
//
//----------------------------------------------------------------------------
void*                               // The allocated storage (never nullptr)
   BlockAllocator::get(             // Allocate storage
     size_t            size)        // Of this length
{
   //-------------------------------------------------------------------------
   // Validate size parameter
   if( size != 0 && size != this->size ) {
     size += (ALIGN - 1);
     size &= ~(ALIGN - 1);
     if( size != this->size )
       throw std::invalid_argument("BlockAllocator::get(size)");
   }

   //-------------------------------------------------------------------------
   // Allocate block from fast array (Lock free)
   for(unsigned i= 0; i<DIM; i++) { // Try to allocate from quick slot
     void* result= fast[i].load();
     if( result && fast[i].compare_exchange_strong(result, nullptr) )
       return result;
   }

   //-------------------------------------------------------------------------
   // Allocate block from free list (Latch required)
   //     (compare_exchange still required for put concurrency)
   std::lock_guard<decltype(mutex)> lock(mutex);

   void* result= free.load();
   while( result ) {
     void* next= ((Free*)result)->next;
     if( free.compare_exchange_strong(result, next) )
       return result;
   }

   //-------------------------------------------------------------------------
   // No storage available: Allocate and format a new block
   size_t b_size= this->b_size;
   char* alloc= (char*)::malloc(b_size);
   if( alloc == nullptr )
     throw std::bad_alloc();

   b_list.lifo((Block*)alloc);      // Add the Block to the allocated List
   alloc  += sizeof(Block);
   b_size -= sizeof(Block);

   result= alloc;                   // Allocate the first block
   alloc  += size;
   b_size -= size;
   while( b_size > size ) {
     put(alloc);                    // Add the block to the free list

     alloc  += size;
     b_size -= size;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       BlockAllocator::put
//
// Purpose-
//       Release storage
//
// Implementation note-
//       Storage release is lock-free.
//
//----------------------------------------------------------------------------
void
   BlockAllocator::put(             // Deallocate
     void*             addr,        // This storage
     size_t            size)        // Of this length
{
   //-------------------------------------------------------------------------
   // Validate size parameter
   if( size != 0 && size != this->size ) {
     size += (ALIGN - 1);           // Round up to alignment
     size &= ~(ALIGN - 1);
     if( size != this->size )
       throw std::invalid_argument("BlockAllocator::put(size)");
   }

   //-------------------------------------------------------------------------
   // Insert block onto slot list
   for(unsigned i= 0; i<DIM; i++) {
     void* old= fast[i].load();
     if( old == nullptr ) {
       if( fast[i].compare_exchange_strong(old, addr) )
         return;
     }
   }

   //-------------------------------------------------------------------------
   // Fast table full, add to list
   void* next= free.load();
   ((Free*)addr)->next= (Free*)next;
   while( ! free.compare_exchange_weak(next, addr) )
     ((Free*)addr)->next= (Free*)next;
}
}  // namespace pub
