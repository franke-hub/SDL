//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Memtest0.cpp
//
// Purpose-
//       Memory test.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <atomic>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/define.h>
#include <com/Random.h>
#include "Test_Mem.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "MEMTEST0" // Source file

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
unsigned             quiet= 0;      // Non-zero for quiet test

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
Random               rng;           // Random number generator

//----------------------------------------------------------------------------
// Static constants
//----------------------------------------------------------------------------
#define MEMARRAY 9
static unsigned      memValue[MEMARRAY]= { // Memory value array
   0x00000000,
   0xffffffff,
   0x00000000,
   0x0f0f0f0f,
   0xf0f0f0f0,
   0xffffffff,
   0x55555555,
   0xaaaaaaaa,
   0x00000000,
   };

static const char*   errValue[MEMARRAY]= { // Memory error array
   "Initial ******** => 00000000",
   "Stuck 0 00000000 => ffffffff",
   "Stuck 1 ffffffff => 00000000",
   "Stuck 0 00000000 => 0f0f0f0f",
   "Flip    0f0f0f0f => f0f0f0f0",
   "Stuck 0 f0f0f0f0 => ffffffff",
   "Stuck 1 ffffffff => 55555555",
   "Flip    55555555 => aaaaaaaa",
   "Stuck 1 aaaaaaaa => 00000000",
   };

//----------------------------------------------------------------------------
//
// Subroutine-
//       setAddr0
//
// Purpose-
//       Set each word in a memory range to a value.
//
//----------------------------------------------------------------------------
static void
   setAddr0(                        // Set Range
     unsigned*         addr,        // Region address
     size_t            size,        // Region size
     unsigned          value)       // Index value
{
   size_t              words= size / sizeof(unsigned);
   unsigned            want;

   //-------------------------------------------------------------------------
   // Set the memory range
   //-------------------------------------------------------------------------
   for(size_t i= 0; i<words; i++)
   {
     want= (unsigned)((uintptr_t)addr << value) | ((uintptr_t)addr >> (32-value));
     *addr= want;
     addr++;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verAddr0
//
// Purpose-
//       Verify that each word in a memory range contains a specific value.
//
//----------------------------------------------------------------------------
static int                          // TRUE if valid
   verAddr0(                        // Verify range
     void*             inpAddr,     // Region address
     unsigned          size,        // Region size
     unsigned          value)       // Word value
{
   unsigned*           addr= (unsigned*)inpAddr;
   size_t              words= size / sizeof(unsigned);
   unsigned            temp;
   unsigned            want;

   //-------------------------------------------------------------------------
   // Verify the memory range
   //-------------------------------------------------------------------------
   #ifdef SCDM
     printf("\r");
   #endif
   for(size_t i= 0; i<words; i++)
   {
     want= (unsigned)((uintptr_t)addr << value) | ((uintptr_t)addr >> (32-value));
     temp= *addr;

     #ifdef SCDM
       printf("A[%p] V[%.8X]\r", addr, want);
     #endif

     if( temp != want )
     {
       fprintf(stderr, "[%p] [%.8zx] Contains(%.8x) Not(%.8x)\n",
                        addr,
                        i*sizeof(unsigned),
                        temp,
                        want);
       return FALSE;
     }

     addr++;
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       setAddr1
//
// Purpose-
//       Set each word in a memory range to a value.
//
//----------------------------------------------------------------------------
static void
   setAddr1(                        // Set Range
     unsigned*         addr,        // Region address
     size_t            size,        // Region size
     unsigned          value)       // Index value
{
   size_t              words= size / sizeof(unsigned);
   unsigned            want;

   //-------------------------------------------------------------------------
   // Set the memory range
   //-------------------------------------------------------------------------
   for(size_t i= 0; i<words; i++)
   {
     want= (unsigned)((uintptr_t)addr << value) | ((uintptr_t)addr >> (32-value));
     want= ~want;
     *addr= want;
     addr++;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verAddr1
//
// Purpose-
//       Verify that each word in a memory range contains a specific value.
//
//----------------------------------------------------------------------------
static int                          // TRUE if valid
   verAddr1(                        // Verify range
     unsigned*         addr,        // Region address
     size_t            size,        // Region size
     unsigned          value)       // Word value
{
   size_t              words= size / sizeof(unsigned);
   unsigned            temp;
   unsigned            want;

   //-------------------------------------------------------------------------
   // Verify the memory range
   //-------------------------------------------------------------------------
   #ifdef SCDM
     printf("\r");
   #endif
   for(size_t i= 0; i<words; i++)
   {
     want= (unsigned)((uintptr_t)addr << value) | ((uintptr_t)addr >> (32-value));
     want= ~want;
     temp= (*addr);

     #ifdef SCDM
       printf("A[%p] V[%.8X]\r", addr, want);
     #endif

     if( temp != want )
     {
       fprintf(stderr, "[%p] [%.8zx] Contains(%.8x) Not(%.8x)\n",
                        addr,
                        i*sizeof(unsigned),
                        temp,
                        want);
       return FALSE;
     }

     addr++;
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       setAtomic
//
// Purpose-
//       Atomically set each word in a memory range to a value.
//
//----------------------------------------------------------------------------
static void
   setAtomic(                       // Set Range
     unsigned*         inpAddr,     // Region address
     size_t            size,        // Region size
     unsigned          oldValue,    // Old value
     unsigned          newValue)    // New value
{
   std::atomic<unsigned>* addr= (std::atomic<unsigned>*)inpAddr;
   size_t              words= size / sizeof(unsigned);

   //-------------------------------------------------------------------------
   // Set the memory range
   //-------------------------------------------------------------------------
   for(size_t i= 0; i<words; i++)
   {
     unsigned repValue= oldValue;
     if( addr->compare_exchange_strong(repValue, newValue) == 0 )
     {
       fprintf(stderr, "\n%s %4d: setAtomic(%p[%.8zx],%.8x,%.8x) contains(%.8x)\n",
                       __SOURCE__, __LINE__,
                       inpAddr, i, oldValue, newValue, repValue);
     }
     addr++;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verAtomic
//
// Purpose-
//       Verify that each word in a memory range contains a specific value.
//
//----------------------------------------------------------------------------
static int                          // TRUE if valid
   verAtomic(                       // Verify range
     unsigned*         inpAddr,     // Region address
     size_t            size,        // Region size
     unsigned          oldValue,    // Old value
     unsigned          newValue)    // New value
{
   std::atomic<unsigned>* addr= (std::atomic<unsigned>*)inpAddr;
   size_t              words= size / sizeof(unsigned);
   unsigned            repValue;

   //-------------------------------------------------------------------------
   // Verify the memory range
   //-------------------------------------------------------------------------
   for(size_t i= 0; i<words; i++)
   {
     repValue= addr->load();
     if( repValue != newValue )
     {
       fprintf(stderr, "%s %4d: verAtomic(%p[%.8zx],%.8x,%.8x) contains(%.8x)\n",
                       __SOURCE__, __LINE__,
                       inpAddr, i, oldValue, newValue, repValue);
       return FALSE;
     }

     addr++;
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       setRandom
//
// Purpose-
//       Set each word in a memory range to a random value.
//
//----------------------------------------------------------------------------
static void
   setRandom(                       // Set Range
     unsigned*         addr,        // Region address
     size_t            size)        // Region size
{
   size_t              words= size / sizeof(unsigned);

   //-------------------------------------------------------------------------
   // Set the memory range
   //-------------------------------------------------------------------------
   for(size_t i= 0; i<words; i++)
   {
     *addr= rng.get();
     addr++;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verRandom
//
// Purpose-
//       Verify that each word in a memory range contains a specific value.
//
//----------------------------------------------------------------------------
static int                          // TRUE if valid
   verRandom(                       // Verify range
     unsigned*         addr,        // Region address
     size_t            size)        // Region size
{
   size_t              words= size / sizeof(unsigned);
   unsigned            value;       // Word value
   unsigned            temp;

   //-------------------------------------------------------------------------
   // Verify the memory range
   //-------------------------------------------------------------------------
   for(size_t i= 0; i<words; i++)
   {
     temp= (*addr);
     value= rng.get();
     if( temp != value )
     {
       fprintf(stderr, "[%p] [%.8zx] Contains(%.8x) Not(%.8x)\n",
                        addr,
                        i*sizeof(unsigned),
                        temp,
                        value);
       return FALSE;
     }

     addr++;
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       setValue
//
// Purpose-
//       Set each word in a memory range to a value.
//
//----------------------------------------------------------------------------
static void
   setValue(                        // Set Range
     unsigned*         addr,        // Region address
     size_t            size,        // Region size
     unsigned          value)       // Word value
{
   size_t              words= size / sizeof(unsigned);

   //-------------------------------------------------------------------------
   // Set the memory range
   //-------------------------------------------------------------------------
   for(size_t i= 0; i<words; i++)
   {
     *addr= value;
     addr++;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verValue
//
// Purpose-
//       Verify that each word in a memory range contains a specific value.
//
//----------------------------------------------------------------------------
static int                          // TRUE if valid
   verValue(                        // Verify range
     unsigned*         addr,        // Region address
     size_t            size,        // Region size
     unsigned          value)       // Word value
{
   size_t              words= size / sizeof(unsigned);
   unsigned            temp;

   //-------------------------------------------------------------------------
   // Verify the memory range
   //-------------------------------------------------------------------------
   for(size_t i= 0; i<words; i++)
   {
     temp= *addr;
     if( temp != value )
     {
       fprintf(stderr, "[%p] [%.8zx] Contains(%.8x) Not(%.8x)\n",
                        addr,
                        i*sizeof(unsigned),
                        temp,
                        value);
       return FALSE;
     }

     addr++;
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       memtest0
//
// Purpose-
//       Memory test.
//
//----------------------------------------------------------------------------
extern void
   memtest0(                        // Memory test
     unsigned*         addr,        // Region address
     size_t            size)        // Region size
{
   unsigned            oldValue;
   unsigned            newValue;

   //-------------------------------------------------------------------------
   // Memory test
   //-------------------------------------------------------------------------
   if( !quiet)
     fprintf(stderr, " Pattern");
   for(int i= 0; i<MEMARRAY; i++)
   {
     setValue(addr, size, memValue[i]);
     if( !verValue(addr, size, memValue[i]) )
     {
       fprintf(stderr, "Error(%s)\n", errValue[i]);
       return;
     }

     if( !quiet)
     {
       fprintf(stderr, ".");
       fflush(stderr);
     }
   }

   if( !quiet)
     fprintf(stderr, "\n");

   //-------------------------------------------------------------------------
   // Randomized Memory test
   //-------------------------------------------------------------------------
   if( !quiet)
     fprintf(stderr, "  Random");
   for(int i= 0; i<MEMARRAY; i++)
   {
     rng.setSeed(memValue[i]);
     setRandom(addr, size);

     rng.setSeed(memValue[i]);
     if( !verRandom(addr, size) )
       return;

     if( !quiet)
     {
       fprintf(stderr, ".");
       fflush(stderr);
     }
   }

   if( !quiet)
     fprintf(stderr, "\n");

   //-------------------------------------------------------------------------
   // Addressing Memory test
   //-------------------------------------------------------------------------
   if( !quiet)
     fprintf(stderr, " Address");
   for(int i= 0; i<32; i++)
   {
     setAddr0(addr, size, i);
     if( !verAddr0(addr, size, i) )
       return;

     if( !quiet)
     {
       fprintf(stderr, ".");
       fflush(stderr);
     }
   }

   if( !quiet)
     fprintf(stderr, "\n");

   //-------------------------------------------------------------------------
   // Addressing Memory test
   //-------------------------------------------------------------------------
   if( !quiet)
     fprintf(stderr, "!Address");
   for(int i= 0; i<32; i++)
   {
     setAddr1(addr, size, i);
     if( !verAddr1(addr, size, i) )
       return;

     if( !quiet)
     {
       fprintf(stderr, ".");
       fflush(stderr);
     }
   }

   if( !quiet)
     fprintf(stderr, "\n");

   //-------------------------------------------------------------------------
   // Atomic Memory test
   //-------------------------------------------------------------------------
   if( !quiet)
     fprintf(stderr, "  Atomic");
   memset(addr, 0, size);
   newValue= 0;
   for(int i= 0; i<MEMARRAY; i++)
   {
     oldValue= newValue;
     newValue= memValue[i];

     setAtomic(addr, size, oldValue, newValue);
     if( !verAtomic(addr, size, oldValue, newValue) )
       return;

     if( !quiet)
     {
       fprintf(stderr, ".");
       fflush(stderr);
     }
   }

   if( !quiet)
     fprintf(stderr, "\n");
}

