//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       SMS_Pool.cpp
//
// Purpose-
//       SMS test: Test Pool object.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <new>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/Pool.h>
#include <com/syslib.h>

#include "Test_SMS.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "SMS_POOL" // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define HCDM                        // If defined, Hard Core Debug Mode
#define SCDM                        // If defined, Soft Core Debug Mode

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define SUBPOOLS                 16 // Number of subpools
#define FREESPACE        0X00010000 // Amount of free space per Pool

//----------------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------------
struct Imp
{
   Pool              pool[SUBPOOLS];// Pools
};

//----------------------------------------------------------------------------
//
// Method-
//       Test_SMS::Test_SMS
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Test_SMS::Test_SMS( void )       // Constructor
:
   imp(NULL)
{
   Imp*              ptrImp;

   int               i;

   ptrImp= (Imp*)malloc(sizeof(Imp));
   if( ptrImp == NULL )
   {
     debugf("%s %d: No storage\n", __SOURCE__, __LINE__);
     abort();
   }
   imp= ptrImp;
   memset((char*)ptrImp, 0, sizeof(Imp));

   for(i=0; i<SUBPOOLS; i++)
   {
     new (&ptrImp->pool[i]) Pool();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Test_SMS::~Test_SMS
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Test_SMS::~Test_SMS( void )      // Destructor
{
   Imp*              ptrImp= (Imp*)imp;

   int               i;

   if( ptrImp == NULL )
     return;

   for(i=0; i<SUBPOOLS; i++)
   {
     ptrImp->pool[i].~Pool();
   }

   free(ptrImp);
   imp= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Test_SMS::debug
//
// Purpose-
//       Debug storage.
//
//----------------------------------------------------------------------------
void
   Test_SMS::debug( void ) const    // Debug storage
{
   Imp*              ptrImp= (Imp*)imp;

   int               i;

   for(i=0; i<SUBPOOLS; i++)
   {
     ptrImp->pool[i].diagnosticDump();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Test_SMS::allocate
//
// Purpose-
//       Allocate storage from subpool.
//
//----------------------------------------------------------------------------
void*                               // -> Allocated storage
   Test_SMS::allocate(              // Allocate storage
     unsigned long   size,          // Required length
     unsigned        subpool)       // Required subpool
{
   Imp*              ptrImp= (Imp*)imp;

   return ptrImp->pool[subpool].allocate(size);
}

//----------------------------------------------------------------------------
//
// Method-
//       Test_SMS::allocate
//
// Purpose-
//       Allocate storage.
//
//----------------------------------------------------------------------------
void*                               // -> Allocated storage
   Test_SMS::allocate(              // Allocate storage
     unsigned long   size)          // Required length (rounded)
{
   Imp*              ptrImp= (Imp*)imp;

   return ptrImp->pool[0].allocate(size);
}

//----------------------------------------------------------------------------
//
// Method-
//       Test_SMS::release
//
// Purpose-
//       Release storage from subpool.
//
//----------------------------------------------------------------------------
void
   Test_SMS::release(               // Release storage
     void*           addr,          // Release address
     unsigned long   size,          // Required length
     unsigned        subpool)       // Required subpool
{
   Imp*              ptrImp= (Imp*)imp;

   ptrImp->pool[subpool].release(addr, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       Test_SMS::release
//
// Purpose-
//       Release storage.
//
//----------------------------------------------------------------------------
void
   Test_SMS::release(               // Release storage
     void*           addr,          // Release address
     unsigned long   size)          // Release length
{
   Imp*              ptrImp= (Imp*)imp;

   ptrImp->pool[0].release(addr, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       Test_SMS::release
//
// Purpose-
//       Release subpool.
//
//----------------------------------------------------------------------------
void
   Test_SMS::release(               // Release subpool
     unsigned        subpool)       // Required subpool
{
   Imp*              ptrImp= (Imp*)imp;

   ptrImp->pool[subpool].reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       Test_SMS::getUsed
//
// Purpose-
//       Determine how many bytes are allocated.
//
//----------------------------------------------------------------------------
unsigned long                       // The number of allocated bytes
   Test_SMS::getUsed( void ) const  // Get number of allocated bytes
{
   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Test_SMS::getMaxSize
//
// Purpose-
//       Determine the largest allocation size.
//
//----------------------------------------------------------------------------
unsigned long                       // The maximum allocation size
   Test_SMS::getMaxSize( void ) const // Get maximum allocation size
{
   return FREESPACE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Test_SMS::getMinSize
//
// Purpose-
//       Determine the smallest allocation size.
//
//----------------------------------------------------------------------------
unsigned long                       // The minimum allocation size
   Test_SMS::getMinSize( void ) const // Get minimum allocation size
{
   return 1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Test_SMS::getSubpools
//
// Purpose-
//       Determine how many subpools (if any) are supported.
//
// Notes-
//       Returns 0 if subpool release is not supported.
//
//----------------------------------------------------------------------------
long                                // The number of supported subpools
   Test_SMS::getSubpools( void ) const // Get number of supported subpools
{
#if 1
   return 16;
#else
   return 0;
#endif
}

