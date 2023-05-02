//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Subpool.cpp
//
// Purpose-
//       Storage Subpool (Allocate only.)
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

#include <com/Debug.h>
#include <com/Unconditional.h>
#include "com/Subpool.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define LINK_SIZE      0x000ffff8   // Nominal PoolLink allocation size
#define ROUND          8            // Element size rounding factor

//----------------------------------------------------------------------------
//
// Subroutine-
//       no_storage
//
// Purpose-
//       Handle storage allocation failure.
//
//----------------------------------------------------------------------------
static inline void
   no_storage(                      // Handle storage allocation failure
    size_t             size)        // Storage allocation length
{
   debugf("Subpool::allocate(%zd): no storage\n", size);
   throw std::runtime_error("no storage");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       round
//
// Purpose-
//       Round up size to next multiple of 8.
//
//----------------------------------------------------------------------------
#undef round                        // Used as a function
static inline size_t                // Rounded size
   round(                           // Round size
     size_t            size)        // Request size
{
   size += size_t(ROUND-1);
   size &= size_t(-(ROUND));

   return size;
}

//----------------------------------------------------------------------------
//
// Struct-
//       Subpool::PoolLink
//
// Purpose-
//       Internal storage allocation block.
//
//----------------------------------------------------------------------------
struct Subpool::PoolLink {
//----------------------------------------------------------------------------
// Subpool::PoolLink::Attributes
//----------------------------------------------------------------------------
PoolLink*              next;        // -> Next PoolLink
size_t                 used;        // Number of bytes used

//----------------------------------------------------------------------------
// Subpool::PoolLink::Methods
//----------------------------------------------------------------------------
inline char*                        // -> Allocated storage
   allocate(                        // Allocate from PoolLink
     size_t            size)        // Request length (rounded)
{
   char*               resultant;   // Resultant

   if( ((LINK_SIZE)-used) < size )
     return nullptr;

   resultant= (char*)this + used;
   used += size;
   return resultant;
}

void
   diagnosticDump( void ) const     // Diagnostic storage dump
{
   tracef("Subpool::PoolLink(%p)::diagnosticDump() next(%p) used(%zd)\n",
          this, next, used);
   dump(this, used);
}
};

//----------------------------------------------------------------------------
//
// Method-
//       traceAllocate
//
// Purpose-
//       Trace allocation (HCDM only)
//
//----------------------------------------------------------------------------
inline void
   traceAllocate(                   // Trace allocation
     Subpool*          pool,        // Associated pool
     void*             addr,        // Returned address
     unsigned long     size)        // Request length (actual)
{
#ifdef HCDM
   tracef("%p= Subpool(%p)::allocate(%lu)\n", addr, pool, size);
#else                               // Parameters only used if HCDM defined
   (void)pool;
   (void)addr;
   (void)size;
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Subpool::~Subpool( void )
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Subpool::~Subpool( void )
{
   release();
}

//----------------------------------------------------------------------------
//
// Method-
//       Subpool::Subpool( void )
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Subpool::Subpool( void )
:  head(nullptr)
,  tail(nullptr)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Subpool::diagnosticDump
//
// Purpose-
//       Diagnostic storage dump.
//
//----------------------------------------------------------------------------
void
   Subpool::diagnosticDump( void ) const // Diagnostic storage dump
{
   debugf("Subpool(%p)::diagnosticDump head(%p) tail(%p)\n", this,
          head, tail);

   size_t used= 0;
   size_t left= 0;
   for(PoolLink* ptrLink= head; ptrLink != nullptr; ptrLink= ptrLink->next)
   {
     ptrLink->diagnosticDump();
     used += ptrLink->used;
     if( ptrLink->used < LINK_SIZE )
       left += ((LINK_SIZE) - ptrLink->used);
   }

   debugf("Used(%zd) Left(%zd)\n", used, left);
}

//----------------------------------------------------------------------------
//
// Method-
//       Subpool::allocate
//
// Purpose-
//       Allocate storage from Subpool.
//
//----------------------------------------------------------------------------
void*                               // Allocated storage
   Subpool::allocate(               // Allocate storage
     size_t            size)        // Required length
{
   char*               resultant;

   size_t actual_size= round(size);
   if( actual_size < size )
     no_storage(size);

   // If not initialized, initialize initial block now
   if( head == nullptr )
   {
     head= tail= (PoolLink*)Unconditional::malloc(LINK_SIZE);
     head->next= nullptr;
     head->used= round(sizeof(PoolLink));
   }

   // Blocks larger than PoolLink are allocated stand-alone
   if( actual_size > ((LINK_SIZE) - round(sizeof(PoolLink))) )
   {
     actual_size += round(sizeof(PoolLink));
     if( actual_size < size )
       no_storage(size);

     PoolLink* ptrLink= (PoolLink*)Unconditional::malloc(actual_size);
     ptrLink->next= nullptr;
     ptrLink->used= actual_size;

     // Add to end of list
     tail->next= ptrLink;
     ptrLink= tail;

     // Return allocated storage
     resultant= (char*)ptrLink + round(sizeof(PoolLink));
     return resultant;
   }

   // Allocate from pool
   unsigned count= 0;               // (Used to limit search length)
   for(PoolLink* ptrLink= head; ptrLink != nullptr; ptrLink= ptrLink->next)
   {
     resultant= ptrLink->allocate(actual_size);
     if( resultant != nullptr )
     {
       traceAllocate(this, resultant, size);
       return resultant;
     }

     if( ++count > 3 )
       break;
   }

   // Allocate new PoolLink and allocate from it.
   PoolLink* ptrLink= (PoolLink*)Unconditional::malloc(LINK_SIZE);
   ptrLink->next= head;
   ptrLink->used= round(sizeof(PoolLink));
   head= ptrLink;

   resultant= ptrLink->allocate(actual_size); // (MUST SUCCEED)
   traceAllocate(this, resultant, size);
   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       Subpool::release
//
// Purpose-
//       Release entire Subpool.
//
//----------------------------------------------------------------------------
void
   Subpool::release( void )         // Release entire Subpool.
{
// debugf("Subpool::release()\n");

   PoolLink* ptrLink= head;
   while(ptrLink != nullptr )
   {
     head= ptrLink->next;
     delete ptrLink;

     ptrLink= head;
   }

   tail= nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       Subpool::strdup
//
// Purpose-
//       Duplicate string using Subpool storage.
//
//----------------------------------------------------------------------------
char*                               // The duplicated string
   Subpool::strdup(                 // Duplicate
     const char*       inp)         // This string
{
   size_t L= strlen(inp);
   char* out= (char*)allocate(L+1);
   ::strcpy(out, inp);

   return out;
}

