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
//       SMS_base.cpp
//
// Purpose-
//       SMS test using malloc/free.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/define.h>
#include <com/syslib.h>

#include "Test_SMS.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "SMS_base" // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define HCDM                        // If defined, Hard Core Debug Mode
#define SCDM                        // If defined, Soft Core Debug Mode

//----------------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------------
struct Imp
{
   unsigned long     used;          // Number of bytes used
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

   ptrImp= (Imp*)malloc(sizeof(Imp));
   if( ptrImp == NULL )
   {
     fprintf(stderr, "%s %d: No storage\n", __SOURCE__, __LINE__);
     abort();
   }
   imp= ptrImp;

   memset(ptrImp, 0, sizeof(Imp));
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
   free(imp);

   imp= NULL;
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
     unsigned long   size)          // Required length
{
   Imp*              ptrImp= (Imp*)imp;

   ptrImp->used += size;
   return malloc(size);
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
   (void)size; (void)subpool;       // Unused parameters

   fprintf(stderr, "%s %d: allocate(subpool) not supported\n",
                   __SOURCE__, __LINE__);
   return NULL;
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
     unsigned long   size)          // Required length
{
   Imp*              ptrImp= (Imp*)imp;

   ptrImp->used -= size;
   free(addr);
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
   (void)addr; (void)size; (void)subpool; // Unused parameters

   fprintf(stderr, "%s %d: release(subpool) not supported\n",
                   __SOURCE__, __LINE__);
   exit(EXIT_FAILURE);
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
   (void)subpool;                   // Unused parameter

   fprintf(stderr, "%s %d: release(subpool) not supported\n",
                   __SOURCE__, __LINE__);
   exit(EXIT_FAILURE);
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
   Imp*              ptrImp= (Imp*)imp;

   return ptrImp->used;
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
   return 4 * 1024 * 1024;
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
   return 0;
}

