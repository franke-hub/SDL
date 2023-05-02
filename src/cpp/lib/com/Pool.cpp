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
//       Pool.cpp
//
// Purpose-
//       Storage allocation from Pool.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include "com/Pool.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "Pool    " // Source file

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class PoolLink;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define FREESPACE        0X00010000 // Amount of free space per Pool
#define FQEINUSE      ((FQEHead*)1) // Special header -- element in use

// The rounding factor must be a power of two and large enough to contain
// the largest of an AQE or FQE head/tail combination
#define ROUND                    32 // Element size rounding factor

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#undef round                        // Used as a function

//----------------------------------------------------------------------------
// Allocated Queue Element Header
//----------------------------------------------------------------------------
struct AQEHead
{
   unsigned long       size;        // Constant zero
   PoolLink*           pool;        // -> PoolLink
};

//----------------------------------------------------------------------------
// Allocated Queue Element Trailer
//----------------------------------------------------------------------------
struct AQETail
{
   unsigned long       size;        // Size of this element
};

//----------------------------------------------------------------------------
// Free Queue Element Header
//----------------------------------------------------------------------------
struct FQEHead
{
   unsigned long       size;        // Size of this element
   FQEHead*            next;        // -> Next free element
   FQEHead*            prev;        // -> Prior free element
};

//----------------------------------------------------------------------------
// Free Queue Element Trailer
//----------------------------------------------------------------------------
struct FQETail
{
   unsigned long       size;        // Size of this element
};

//----------------------------------------------------------------------------
// PoolLink element
//----------------------------------------------------------------------------
class PoolLink : public List<PoolLink>::Link
{
public:
//----------------------------------------------------------------------------
// PoolLink::Attributes
//----------------------------------------------------------------------------
   unsigned long       ident;       // Identifier
   unsigned long       used;        // Number of bytes used

   FQEHead*            head;        // -> First free element
   FQEHead*            tail;        // -> Last  free element

   AQEHead             topHead;     // Permanently allocated prefix element
   char                top[ROUND - sizeof(AQEHead) - sizeof(AQETail)];
   AQETail             topTail;     // Permanently allocated prefix element

   char                space[FREESPACE]; // Free space

   AQEHead             botHead;     // Permanently allocated suffix element
   char                bot[ROUND - sizeof(AQEHead)];

//----------------------------------------------------------------------------
// PoolLink::Enumerations and typedefs
//----------------------------------------------------------------------------
enum
{
   IDENT=                0xfefe0001 // Identifier
};

//----------------------------------------------------------------------------
// PoolLink::Constructors
//----------------------------------------------------------------------------
   ~PoolLink( void ) {}             // Destructor
   PoolLink( void );                // Constructor

//----------------------------------------------------------------------------
// PoolLink::Methods
//----------------------------------------------------------------------------
inline void*                        // -> Allocated storage
   allocate(                        // Allocate from PoolLink
     unsigned long     size);       // Request length (rounded)

inline void
   release(                         // Release into PoolLink
     AQEHead*          addr,        // Release address
     unsigned long     size);       // Release length (rounded)

static inline unsigned long         // Rounded size
   round(                           // Round size
     unsigned long     size)        // Request size
{
   unsigned long       resultant;   // Rounded size

   resultant= size+sizeof(AQEHead)+sizeof(AQETail)+(ROUND-1);
   resultant &= -(ROUND);

   return resultant;
}

int                                 // TRUE if coherent
   isCoherent( void ) const;        // Is PoolLink coherent?

int                                 // TRUE if element contained
   contains(                        // Is element contained?
     void*             addr) const; // -> Element

void
   diagnosticDump( void ) const;    // Diagnostic storage dump

inline int                          // TRUE if element is valid
   elementIsValid(                  // Check coherency
     FQEHead*          ptrFH,       // -> Element
     FQEHead*          prvFH) const;// -> Prior Element
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
     Pool*             pool,        // Associated pool
     void*             addr,        // Returned address
     unsigned long     size)        // Request length (actual)
{
   #ifdef HCDM
     tracef("%p= Pool(%p)::allocate(%lu)\n", addr, pool, size);
   #else                            // Parameters unused unless HCDM defined
     (void)pool;
     (void)addr;
     (void)size;
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       traceRelease
//
// Purpose-
//       Trace release (HCDM only)
//
//----------------------------------------------------------------------------
inline void
   traceRelease(                    // Trace release
     Pool*             pool,        // Associated pool
     void*             addr,        // Request address
     unsigned long     size)        // Request length (actual)
{
   #ifdef HCDM
     tracef("Pool(%p)::release(%p,%lu)\n", pool, addr, size);
   #else                            // Parameters unused unless HCDM defined
     (void)pool;
     (void)addr;
     (void)size;
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       PoolLink::PoolLink
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   PoolLink::PoolLink( void )       // Constructor
:  List<PoolLink>::Link()
,  ident(IDENT)
,  used(0)
{
   FQEHead*            ptrFH;
   FQETail*            ptrFT;

   // Initialize the (permanently allocated) head element
   topHead.size= 0;
// topHead.pool= this;
// memset(top, 'T', sizeof(top));
   topTail.size= sizeof(AQEHead)+sizeof(top)+sizeof(AQETail);

   // Initialize the free storage chain
   head= (FQEHead*)space;
   tail= (FQEHead*)space;

   ptrFH= head;
   ptrFH->size= FREESPACE;
   ptrFH->next= NULL;
   ptrFH->prev= NULL;

   ptrFT= (FQETail*)((char*)ptrFH+ptrFH->size);
   ptrFT--;
   ptrFT->size= FREESPACE;

   // Initialize the (permanently allocated) tail element
   botHead.size= 0;
// botHead.pool= this;
}

//----------------------------------------------------------------------------
//
// Method-
//       PoolLink::isCoherent
//
// Purpose-
//       Internal coherency check.
//
//----------------------------------------------------------------------------
int                                 // TRUE if coherent
   PoolLink::isCoherent( void ) const // Is PoolLink coherent?
{
   FQEHead*            prvFH;
   FQEHead*            ptrFH;

   unsigned long       maxCount;

   prvFH= NULL;
   ptrFH= head;
   maxCount= FREESPACE;
   while( ptrFH != NULL )
   {
     if( !elementIsValid(ptrFH, prvFH) )
       return FALSE;

     prvFH= ptrFH;
     ptrFH= ptrFH->next;

     maxCount--;
     if( maxCount == 0 )
       return FALSE;
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       PoolLink::contains
//
// Purpose-
//       Validate one element.
//
//----------------------------------------------------------------------------
int                                 // TRUE if element contained
   PoolLink::contains(              // Is element contained?
     void*             addr) const  // -> Element
{
   if( (char*)addr < &space[0]
       ||(char*)addr >= &space[FREESPACE] )
   {
     return FALSE;
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       PoolLink::diagnosticDump
//
// Purpose-
//       Diagnostic storage dump.
//
//----------------------------------------------------------------------------
void
   PoolLink::diagnosticDump( void ) const // Diagnostic storage dump.
{
   FQEHead*            prvFH;
   FQEHead*            ptrFH;

   tracef("%s PoolLink(%p)::diagnosticDump()\n", __SOURCE__, this);
   tracef("Head(%p)  ", head);
   tracef("Tail(%p)  ", tail);
   tracef("Used(%ld)\n", used);
   dump(this, sizeof(*this));

   prvFH= NULL;
   ptrFH= head;
   while( ptrFH != NULL )
   {
     if( !elementIsValid(ptrFH, prvFH) )
       exit(EXIT_FAILURE);

     tracef("%p next(%p) prev(%p) size(%.4lX)\n",
            ptrFH, ptrFH->next, ptrFH->prev, ptrFH->size);

     prvFH= ptrFH;
     ptrFH= ptrFH->next;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       PoolLink::elementIsValid
//
// Purpose-
//       Validate one element.
//
//----------------------------------------------------------------------------
int                                 // TRUE if element is valid
   PoolLink::elementIsValid(        // Check coherency
     FQEHead*          ptrFH,       // -> Element
     FQEHead*          prvFH) const // -> Prior Element
{
   FQETail*            ptrFT;

   if( !contains(ptrFH) )
   {
     debugf("%s Element(%p) out of range\n", __SOURCE__, ptrFH);
     return FALSE;
   }

   ptrFT= (FQETail*)((char*)ptrFH+ptrFH->size);
   ptrFT--;
   if( !contains(ptrFT) )
   {
     debugf("%s Element(%p) invalid size(%.4lX)\n", __SOURCE__,
            ptrFH, ptrFH->size);
     return FALSE;
   }

   if( ptrFH->size != ptrFT->size )
   {
     debugf("%s Element(%p) header/trailer mismatch\n", __SOURCE__,
            ptrFH);
     return FALSE;
   }

   if( ptrFH->next == ptrFH )
   {
     debugf("%s Element(%p) invalid next\n", __SOURCE__,
            ptrFH);
     return FALSE;
   }

   if( ptrFH->next == NULL && tail != ptrFH )
   {
     debugf("%s Element(%p) tail mismatch\n", __SOURCE__,
            ptrFH);
     return FALSE;
   }

   if( ptrFH->prev != prvFH )
   {
     debugf("%s Element(%p) invalid prev\n", __SOURCE__,
            ptrFH);
     return FALSE;
   }

   if( ptrFH->prev == NULL && head != ptrFH )
   {
     debugf("%s Element(%p) head mismatch\n", __SOURCE__,
            ptrFH);
     return FALSE;
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       PoolLink::allocate
//
// Purpose-
//       Allocate element from PoolLink.
//
//----------------------------------------------------------------------------
inline void*                        // -> Allocated storage
   PoolLink::allocate(              // Allocate from PoolLink
     unsigned long     size)        // Request length (rounded)
{
   AQEHead*            ptrAH;
   AQETail*            ptrAT;
   FQEHead*            altFH;
   FQEHead*            prvFH;
   FQEHead*            ptrFH;
   FQETail*            ptrFT;

   ptrFH= head;
   while( ptrFH != NULL )
   {
     if( ptrFH->size >= size )
       goto found;

     ptrFH= ptrFH->next;
   }
   return NULL;

found:
   // Move skipped elements to end of list
   prvFH= ptrFH->prev;
   if( prvFH != NULL )
   {
     altFH= head;
     head= ptrFH;
     ptrFH->prev= NULL;

     tail->next= altFH;
     altFH->prev= tail;

     prvFH->next= NULL;
     tail= prvFH;
   }

   // Allocate from element
   if( ptrFH->size == size )
   {
     altFH= ptrFH->prev;
     if( altFH == NULL )
       head= ptrFH->next;
     else
       altFH->next= ptrFH->next;

     altFH= ptrFH->next;
     if( altFH == NULL )
       tail= ptrFH->prev;
     else
       altFH->prev= ptrFH->prev;
   }
   else
   {
     ptrFH->size -= size;
     ptrFT= (FQETail*)((char*)ptrFH + ptrFH->size);
     ptrFT--;
     ptrFT->size= ptrFH->size;

     ptrFH= (FQEHead*)((char*)ptrFH + ptrFH->size);
   }

   ptrAH= (AQEHead*)ptrFH;
   ptrAH->size= 0;
   ptrAH->pool= this;
   ptrAT= (AQETail*)((char*)ptrAH + size);
   ptrAT--;
   ptrAT->size= size;

   used += size;
   ptrAH++;

   return ptrAH;
}

//----------------------------------------------------------------------------
//
// Method-
//       PoolLink::release
//
// Purpose-
//       Release storage.
//
//----------------------------------------------------------------------------
inline void
   PoolLink::release(               // Release storage
     AQEHead*          addr,        // Release address
     unsigned long     size)        // Release length (rounded)
{
   AQEHead*            ptrAH;
   AQETail*            ptrAT;
   FQEHead*            altFH;
   FQEHead*            ptrFH;
   FQETail*            ptrFT;

   ptrAH= addr;
   ptrAT= (AQETail*)((char*)ptrAH + size);
   ptrAT--;
   if( ptrAT->size != size
       || used < size )
   {
     debugf("%s %d: Release(%p:%lu) Corrupt trailer(%p)\n",
            __SOURCE__, __LINE__, ptrAH, size, ptrAT);
     diagnosticDump();
     exit(EXIT_FAILURE);
   }

   // Concatenate with prior element
   ptrFT= (FQETail*)ptrAH;
   ptrFT--;
   ptrFH= (FQEHead*)((char*)ptrAH - ptrFT->size);
   if( ptrFH->size != 0 )
   {
     if( ptrFH->size != ptrFT->size )
     {
       debugf("%s %d: Release(%p:%lu) Corrupt prior(%p)\n",
              __SOURCE__, __LINE__, ptrAH, size, ptrFH);
       diagnosticDump();
       exit(EXIT_FAILURE);
     }

     ptrFH->size += size;
     ptrAT->size= ptrFH->size;
   }
   else
   {
     ptrFH= (FQEHead*)ptrAH;
     ptrFH->size= size;

     altFH= tail;
     ptrFH->next= NULL;
     ptrFH->prev= altFH;
     if( altFH != NULL )
       altFH->next= ptrFH;

     tail= ptrFH;
     if( head == NULL )
       head= ptrFH;
   }

   // Concatenate with next element
   altFH= (FQEHead*)((char*)ptrFH + ptrFH->size);
   if( altFH->size != 0 )
   {
     if( altFH->prev == NULL )
       head= altFH->next;
     else
       altFH->prev->next= altFH->next;

     if( altFH->next == NULL )
       tail= altFH->prev;
     else
       altFH->next->prev= altFH->prev;

     ptrFH->size += altFH->size;
     ptrFT= (FQETail*)((char*)ptrFH + ptrFH->size);
     ptrFT--;
     ptrFT->size= ptrFH->size;
   }

   // Account for release
   used -= size;
}

//----------------------------------------------------------------------------
//
// Method-
//       Pool::~Pool( void )
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Pool::~Pool( void )
{
   #ifdef HCDM
     tracef("Pool(%p)::~Pool()\n", this);
   #endif

   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       Pool::Pool( void )
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Pool::Pool( void )
:  List<PoolLink>()
{
   #ifdef HCDM
     tracef("Pool(%p)::Pool()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Pool::isCoherent
//
// Purpose-
//       Examine the Pool for coherency.
//
//----------------------------------------------------------------------------
int                                 // TRUE if coherent
   Pool::isCoherent( void ) const   // Diagnostic coherency check
{
   PoolLink*           ptrLink;

   for(ptrLink= (PoolLink*)head;
       ptrLink != NULL;
       ptrLink= (PoolLink*)ptrLink->getNext())
   {
     if( !ptrLink->isCoherent() )
       return FALSE;
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       Pool::diagnosticDump
//
// Purpose-
//       Diagnostic storage dump.
//
//----------------------------------------------------------------------------
void
   Pool::diagnosticDump( void ) const // Diagnostic storage dump
{
   PoolLink*           ptrLink;

   for(ptrLink= (PoolLink*)head;
       ptrLink != NULL;
       ptrLink= (PoolLink*)ptrLink->getNext())
   {
     ptrLink->diagnosticDump();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Pool::allocate
//
// Purpose-
//       Allocate storage from Pool.
//
//----------------------------------------------------------------------------
void*                               // -> Allocated storage
   Pool::allocate(                  // Allocate storage
     unsigned long     size)        // Required length
{
   PoolLink*           ptrLink;
   void*               resultant;
   unsigned long       actualSize;

   #if 0
     tracef("Pool(%p)::allocate(%d)\n", this, size);
   #endif

   actualSize= PoolLink::round(size);
   if( actualSize < size )
     return NULL;

   for(ptrLink= (PoolLink*)head;
       ptrLink != NULL;
       ptrLink= (PoolLink*)ptrLink->getNext())
   {
     resultant= ptrLink->allocate(actualSize);
     if( resultant != NULL )
     {
       // Move used PoolLink to front
       if( (PoolLink*)getHead() != ptrLink ) // If not already at front
       {
         remove(ptrLink, ptrLink);  // Remove the link
         lifo(ptrLink);             // Add to front
       }

       traceAllocate(this, resultant, size);
       return resultant;
     }
   }

   // Allocate new PoolLink
   ptrLink= new PoolLink();
   if( ptrLink == NULL )
     return NULL;

   lifo(ptrLink);

   // Allocate from new PoolLink
   resultant= ptrLink->allocate(actualSize);
   traceAllocate(this, resultant, size);
   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       Pool::release
//
// Purpose-
//       Release storage into Pool.
//
//----------------------------------------------------------------------------
void
   Pool::release(                   // Release storage
     void*             addr,        // Release address
     unsigned long     size)        // Required length
{
   AQEHead*            ptrAH;
   PoolLink*           ptrLink;
   unsigned long       actualSize;

   traceRelease(this, addr, size);

   actualSize= PoolLink::round(size);
   if( actualSize < size )
     return;

   ptrAH= (AQEHead*)addr;
   ptrAH--;
   ptrLink= ptrAH->pool;
   if( addr <= (void*)ptrLink
       || addr >= (void*)((char*)ptrLink + sizeof(PoolLink))
       || ptrLink->ident != PoolLink::IDENT )
   {
     debugf("%s %d: Release(%p:%lu) Corrupt header(%p)\n",
            __SOURCE__, __LINE__, addr, size, ptrAH);
     diagnosticDump();
     exit(EXIT_FAILURE);
   }

   ptrLink->release(ptrAH, actualSize);

   // If the link is unused and not the first, release it
   if( ptrLink->used == 0
       && (PoolLink*)getHead() != ptrLink )
   {
     remove(ptrLink, ptrLink);
     delete ptrLink;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Pool::reset
//
// Purpose-
//       Release entire Pool.
//
//----------------------------------------------------------------------------
void
   Pool::reset( void )              // Release entire Pool.
{
   PoolLink*           ptrLink;

// debugf("Pool::reset()\n");

   for(;;)
   {
     ptrLink= (PoolLink*)remq();
     if( ptrLink == NULL )
       break;

     delete ptrLink;
   }
}

