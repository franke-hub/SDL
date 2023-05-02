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
//       TestWare.cpp
//
// Purpose-
//       Test Atomic functions, then Hardware and Software objects.
//
// Last change date-
//       2020/10/03
//
// Implementation notes-
//       Functions are not declared static so that compilation completes
//       even when function calls are commented out.
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Clock.h>
#include <com/Debug.h>
#include <com/Exception.h>
#include <com/Interval.h>

#include "com/Atomic.h"
#include "com/Hardware.h"
#include "com/Software.h"
#include "com/Thread.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TestWare" // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#define DIM_ARRAY 10000             // Number of array elements
#define THREAD_COUNT 64             // Number of simultaneous Threads
                                    // Must be 8, 16, 32, or 64

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static long            errorCount= 0;
static char*           testArea;    // Allocated test area
static volatile void*  testPage;    // Allocated test area on page boundary

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class AtomicMpThread;               // The AtomicMpThread object
static void
   atomicMpTest(                    // Run the test
     AtomicMpThread*   thread);     // For this AtomicMpThread

//----------------------------------------------------------------------------
//
// Struct-
//       Pack0, Pack1
//
// Purpose-
//       Packing of byte, half, word, double
//
//----------------------------------------------------------------------------
struct Pack0 {                      // Size= 16
   int8_t              _u00;        // 00 Unused
   ATOMIC8             b;           // 01 Byte
   ATOMIC16            h;           // 02 Half
   ATOMIC32            w;           // 04 Word
   ATOMIC64            d;           // 08 Double
}; // struct Pack0

struct Pack1 {                      // Size= 16
   ATOMIC64            d;           // 00 Double
   ATOMIC32            w;           // 08 Word
   int8_t              _u0C;        // 0C Unused
   ATOMIC8             b;           // 0D Byte
   ATOMIC16            h;           // 0E Half
}; // struct Pack1

//----------------------------------------------------------------------------
//
// Struct-
//       TestArea
//
// Purpose-
//       Allocated test area for Threads.
//
//----------------------------------------------------------------------------
struct TestArea {
   Pack0               storePack0[THREAD_COUNT]; // Simple store test[0]
   Pack1               storePack1[THREAD_COUNT]; // Simple store test[1]

   Pack0               swapPack0[THREAD_COUNT]; // Swap store test[0]
   Pack1               swapPack1[THREAD_COUNT]; // Swap store test[1]

   ATOMIC64            swapD[THREAD_COUNT]; // Double array
   ATOMIC32            swapW[THREAD_COUNT]; // Word array
   ATOMIC16            swapH[THREAD_COUNT]; // Half array
   ATOMIC8             swapB[THREAD_COUNT]; // Byte array

   ATOMIC8             rond08[THREAD_COUNT/8];  // Rondesvous byte array
   ATOMIC16            rond16[THREAD_COUNT/16]; // Rondesvous half array
   ATOMIC32            rond32[THREAD_COUNT/32]; // Rondesvous word array
   ATOMIC64            rondesvous;  // Rondesvous completion word

   AtomicMpThread*     finalHead;   // Completion head
   ATOMICP*            finalTail;   // Completion tail
   signed char         sequence[THREAD_COUNT]; // Completion sequence
}; // struct TestArea

//----------------------------------------------------------------------------
//
// Class-
//       AtomicMpThread
//
// Purpose-
//       Thread which runs atomic multiprocessor tests
//
//----------------------------------------------------------------------------
class AtomicMpThread : public Thread {
   // Attributes
   public:
   int                 index;       // Thread index

   // Constructor
   inline virtual ~AtomicMpThread( void ) {}
   inline AtomicMpThread(int index) : Thread(), index(index) {}

   //-------------------------------------------------------------------------
   // AtomicMpThread::run()
   protected: virtual long run( void )
   {
     int64_t           bitStamp= 1 << index; // Set our bit stamp
     int               cc;          // Compare and Swap resultant
     int               x;           // "Other" AtomicMpThread index

     //-----------------------------------------------------------------------
     // "Simultaneous" blastoff
     Thread::sleep(0.125);          // Everybody waits a little
     while(testPage == NULL )
       Thread::yield();

     TestArea* T= (TestArea*)testPage;
     Thread::yield();

     //-----------------------------------------------------------------------
     // Run the tests
     atomicMpTest(this);

     //-----------------------------------------------------------------------
     // Thread completion check-in sequencer
     AtomicMpThread* oldThread;
     do
     {
       oldThread= (AtomicMpThread*)T->finalTail;
       void* newThread= this;

       cc= csp(&T->finalTail, oldThread, newThread);
     } while( cc != 0 );

     if( oldThread == NULL )
       T->finalHead= this;
     else
     {
       x= oldThread->index;
       T->sequence[x]= index;
     }

     do                             // Rondesvous check-in
     {
       int64_t oldValue= T->rondesvous;
       int64_t newValue= oldValue | bitStamp;

       cc= csd(&T->rondesvous, oldValue, newValue);
     } while( cc != 0 );

     return 0;                      // Always good
   }

   //-------------------------------------------------------------------------
   // AtomicMpThread::runner() ** FOR BRINGUP SINGLE-THREAD TESTING **
   public: virtual void runner( void ) {
     run();
   }
}; // class AtomicMpThread

//----------------------------------------------------------------------------
//
// Subroutine-
//       atomicMpInit
//
// Purpose-
//       Initialize TestArea
//
//----------------------------------------------------------------------------
static void
   atomicMpInit( void )             // Initialize TestArea
{
   testArea= (char*)malloc(sizeof(TestArea) + 4096);
   TestArea* T= (TestArea*)((size_t(testArea) + 4095) & (~4095)); // Rounded area

   // Initialize to zeros (default)
   memset(T, 0, sizeof(TestArea));

   // Initialize to ones
   for(int i= 0; i<THREAD_COUNT; i++)
   {
     T->sequence[i]= (-1);
   }

   // Start testing
   debugf("Waiting...\n");
   Thread::sleep( 5.0);             // Wait for all the threads to initialize

   testPage= T;                     // Release the hounds
   debugf("...Started\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       atomicMpTest
//
// Purpose-
//       Run the test
//
//----------------------------------------------------------------------------
static void
   atomicMpTest(                    // Run the test
     AtomicMpThread*   thread)      // For this AtomicMpThread
{
   const int index= thread->index;  // Get the thread index
   TestArea* T= (TestArea*)testPage;// Address the test page

   // Perform simple stores
   switch( index%32 )
   {
     case 0:
     case 1:
       T->storePack0[index].b= index;
       T->storePack0[index].h= index;
       T->storePack0[index].w= index;
       T->storePack0[index].d= index;

       T->storePack1[index].b= index;
       T->storePack1[index].h= index;
       T->storePack1[index].w= index;
       T->storePack1[index].d= index;
       break;

     case 2:
       T->storePack0[index].b= index;
       T->storePack0[index].h= index;
       T->storePack0[index].d= index;
       T->storePack1[index].w= index;

       T->storePack1[index].b= index;
       T->storePack1[index].h= index;
       T->storePack1[index].d= index;
       T->storePack0[index].w= index;
       break;

     case 3:
       T->storePack0[index].b= index;
       T->storePack0[index].w= index;
       T->storePack1[index].h= index;
       T->storePack0[index].d= index;

       T->storePack1[index].b= index;
       T->storePack1[index].w= index;
       T->storePack0[index].h= index;
       T->storePack1[index].d= index;
       break;

     case 4:
       T->storePack0[index].b= index;
       T->storePack0[index].w= index;
       T->storePack1[index].d= index;
       T->storePack1[index].h= index;

       T->storePack1[index].b= index;
       T->storePack1[index].w= index;
       T->storePack0[index].d= index;
       T->storePack0[index].h= index;
       break;

     case 5:
     case 6:
       T->storePack0[index].b= index;
       T->storePack1[index].d= index;
       T->storePack0[index].h= index;
       T->storePack0[index].w= index;

       T->storePack1[index].b= index;
       T->storePack0[index].d= index;
       T->storePack1[index].h= index;
       T->storePack1[index].w= index;
       break;

     case 7:
       T->storePack0[index].b= index;
       T->storePack1[index].d= index;
       T->storePack0[index].w= index;
       T->storePack1[index].h= index;

       T->storePack1[index].b= index;
       T->storePack0[index].d= index;
       T->storePack1[index].w= index;
       T->storePack0[index].h= index;
       break;

     case 8:
       T->storePack0[index].h= index;
       T->storePack1[index].b= index;
       T->storePack1[index].w= index;
       T->storePack1[index].d= index;

       T->storePack1[index].h= index;
       T->storePack0[index].b= index;
       T->storePack0[index].w= index;
       T->storePack0[index].d= index;
       break;

     case 9:
       T->storePack1[index].h= index;
       T->storePack0[index].b= index;
       T->storePack0[index].d= index;
       T->storePack0[index].w= index;

       T->storePack0[index].h= index;
       T->storePack1[index].b= index;
       T->storePack1[index].d= index;
       T->storePack1[index].w= index;
       break;

     case 10:
     case 11:
       T->storePack1[index].h= index;
       T->storePack0[index].w= index;
       T->storePack0[index].b= index;
       T->storePack1[index].d= index;

       T->storePack0[index].h= index;
       T->storePack1[index].w= index;
       T->storePack1[index].b= index;
       T->storePack0[index].d= index;
       break;

     case 12:
     case 13:
       T->storePack1[index].h= index;
       T->storePack0[index].w= index;
       T->storePack1[index].d= index;
       T->storePack0[index].b= index;

       T->storePack0[index].h= index;
       T->storePack1[index].w= index;
       T->storePack0[index].d= index;
       T->storePack1[index].b= index;
       break;

     case 14:
       T->storePack1[index].h= index;
       T->storePack0[index].d= index;
       T->storePack1[index].b= index;
       T->storePack1[index].w= index;

       T->storePack0[index].h= index;
       T->storePack1[index].d= index;
       T->storePack0[index].b= index;
       T->storePack0[index].w= index;
       break;

     case 15:
       T->storePack1[index].h= index;
       T->storePack1[index].d= index;
       T->storePack0[index].w= index;
       T->storePack0[index].b= index;

       T->storePack0[index].h= index;
       T->storePack0[index].d= index;
       T->storePack1[index].w= index;
       T->storePack1[index].b= index;
       break;

     case 16:
       T->storePack1[index].w= index;
       T->storePack1[index].b= index;
       T->storePack0[index].h= index;
       T->storePack1[index].d= index;

       T->storePack0[index].w= index;
       T->storePack0[index].b= index;
       T->storePack1[index].h= index;
       T->storePack0[index].d= index;
       break;

     case 17:
     case 18:
       T->storePack1[index].w= index;
       T->storePack1[index].b= index;
       T->storePack1[index].d= index;
       T->storePack0[index].h= index;

       T->storePack0[index].w= index;
       T->storePack0[index].b= index;
       T->storePack0[index].d= index;
       T->storePack1[index].h= index;
       break;

     case 19:
       T->storePack1[index].w= index;
       T->storePack1[index].h= index;
       T->storePack1[index].b= index;
       T->storePack1[index].d= index;

       T->storePack0[index].w= index;
       T->storePack0[index].h= index;
       T->storePack0[index].b= index;
       T->storePack0[index].d= index;
       break;

     case 20:
       T->storePack0[index].w= index;
       T->storePack1[index].h= index;
       T->storePack0[index].d= index;
       T->storePack1[index].b= index;

       T->storePack1[index].w= index;
       T->storePack0[index].h= index;
       T->storePack1[index].d= index;
       T->storePack0[index].b= index;
       break;

     case 21:
       T->storePack0[index].w= index;
       T->storePack1[index].d= index;
       T->storePack1[index].b= index;
       T->storePack0[index].h= index;

       T->storePack1[index].w= index;
       T->storePack0[index].d= index;
       T->storePack0[index].b= index;
       T->storePack1[index].h= index;
       break;

     case 22:
     case 23:
       T->storePack1[index].w= index;
       T->storePack0[index].d= index;
       T->storePack0[index].h= index;
       T->storePack1[index].b= index;

       T->storePack0[index].w= index;
       T->storePack1[index].d= index;
       T->storePack1[index].h= index;
       T->storePack0[index].b= index;
       break;

     case 24:
     case 25:
       T->storePack1[index].d= index;
       T->storePack0[index].b= index;
       T->storePack1[index].h= index;
       T->storePack0[index].w= index;

       T->storePack0[index].d= index;
       T->storePack1[index].b= index;
       T->storePack0[index].h= index;
       T->storePack1[index].w= index;
       break;

     case 26:
       T->storePack1[index].d= index;
       T->storePack1[index].b= index;
       T->storePack0[index].w= index;
       T->storePack0[index].h= index;

       T->storePack0[index].d= index;
       T->storePack0[index].b= index;
       T->storePack1[index].w= index;
       T->storePack1[index].h= index;
       break;

     case 27:
       T->storePack0[index].d= index;
       T->storePack0[index].h= index;
       T->storePack0[index].b= index;
       T->storePack0[index].w= index;

       T->storePack1[index].d= index;
       T->storePack1[index].h= index;
       T->storePack1[index].b= index;
       T->storePack1[index].w= index;
       break;

     case 28:
       T->storePack0[index].d= index;
       T->storePack0[index].h= index;
       T->storePack0[index].w= index;
       T->storePack0[index].b= index;

       T->storePack1[index].d= index;
       T->storePack1[index].h= index;
       T->storePack1[index].w= index;
       T->storePack1[index].b= index;
       break;

     case 29:
     case 30:
       T->storePack0[index].d= index;
       T->storePack0[index].w= index;
       T->storePack0[index].b= index;
       T->storePack0[index].h= index;

       T->storePack1[index].d= index;
       T->storePack1[index].w= index;
       T->storePack1[index].b= index;
       T->storePack1[index].h= index;
       break;

     case 31:
       T->storePack0[index].d= index;
       T->storePack0[index].w= index;
       T->storePack0[index].h= index;
       T->storePack0[index].b= index;

       T->storePack1[index].d= index;
       T->storePack1[index].w= index;
       T->storePack1[index].h= index;
       T->storePack1[index].b= index;
       break;

     default:
       throwf("%d %s ShouldNotOccur(%d)\n", __LINE__, __SOURCE__, index);
   }

   // Perform swap stores
   switch( index%32 )
   {
     case 0:
//     debugf("%4d HCDM [%2d] P0(%2d) P1(%2d)\n", __LINE__, index,
//            int(T->swapPack0[index].d),
//            int(T->swapPack1[index].d));

       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );

       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       break;

     case 1:
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );

       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       break;

     case 2:
     case 3:
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );

       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       break;

     case 4:
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );

       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       break;

     case 5:
     case 6:
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );

       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       break;

     case 7:
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );

       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       break;

     case 8:
     case 9:
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );

       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       break;

     case 10:
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );

       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       break;

     case 11:
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );

       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       break;

     case 12:
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );

       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       break;

     case 13:
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );

       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       break;

     case 14:
     case 15:
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );

       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       break;

     case 16:
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );

       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       break;

     case 17:
     case 18:
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );

       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       break;

     case 19:
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );

       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       break;

     case 20:
     case 21:
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );

       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       break;

     case 22:
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );

       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       break;

     case 23:
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );

       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       break;

     case 24:
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );

       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       break;

     case 25:
     case 26:
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );

       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       break;

     case 27:
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );

       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       break;

     case 28:
     case 29:
       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );

       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       break;

     case 30:
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );

       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       break;

     case 31:
       assert( csd(&T->swapPack0[index].d, 0, index) == 0 );
       assert( csw(&T->swapPack0[index].w, 0, index) == 0 );
       assert( csh(&T->swapPack1[index].h, 0, index) == 0 );
       assert( csb(&T->swapPack0[index].b, 0, index) == 0 );

       assert( csd(&T->swapPack1[index].d, 0, index) == 0 );
       assert( csw(&T->swapPack1[index].w, 0, index) == 0 );
       assert( csh(&T->swapPack0[index].h, 0, index) == 0 );
       assert( csb(&T->swapPack1[index].b, 0, index) == 0 );
       break;

     default:
       throwf("%d %s ShouldNotOccur(%d)\n", __LINE__, __SOURCE__, index);
   }

   // Perform rondesvous array swaps
   int cc;
   int arrayIndex;
   int arrayStamp;

   arrayIndex= index/8;
   arrayStamp= 1 << (index%8);
   do
   {
     int oldValue= T->rond08[arrayIndex];
     int newValue= oldValue | arrayStamp;
     assert( (oldValue & arrayStamp) == 0 );

     cc= csb(&T->rond08[arrayIndex], oldValue, newValue);
   } while( cc != 0 );

   arrayIndex= index/16;
   arrayStamp= 1 << (index%16);
   do
   {
     int oldValue= T->rond16[arrayIndex];
     int newValue= oldValue | arrayStamp;
     assert( (oldValue & arrayStamp) == 0 );

     cc= csh(&T->rond16[arrayIndex], oldValue, newValue);
   } while( cc != 0 );

   arrayIndex= index/32;
   arrayStamp= 1 << (index%32);
   do
   {
     int oldValue= T->rond32[arrayIndex];
     int newValue= oldValue | arrayStamp;
     assert( (oldValue & arrayStamp) == 0 );

     cc= csw(&T->rond32[arrayIndex], oldValue, newValue);
   } while( cc != 0 );

   //-------------------------------------------------------------------------
   // Verify swap boundary alignments
   assert( csb(&T->swapB[index], 0, index) == 0 );
   assert( csh(&T->swapH[index], 0, index) == 0 );
   assert( csw(&T->swapW[index], 0, index) == 0 );
   assert( csd(&T->swapD[index], 0, index) == 0 );
   if( index != 0 )
   {
     assert( csb(&T->swapPack0[index].b, 0, index) != 0 );
     assert( csh(&T->swapPack0[index].h, 0, index) != 0 );
     assert( csw(&T->swapPack0[index].w, 0, index) != 0 );
     assert( csd(&T->swapPack0[index].d, 0, index) != 0 );

     assert( csb(&T->swapPack1[index].b, 0, index) != 0 );
     assert( csh(&T->swapPack1[index].h, 0, index) != 0 );
     assert( csw(&T->swapPack1[index].w, 0, index) != 0 );
     assert( csd(&T->swapPack1[index].d, 0, index) != 0 );

     assert( csb(&T->swapB[index], 0, index) != 0 );
     assert( csh(&T->swapH[index], 0, index) != 0 );
     assert( csw(&T->swapW[index], 0, index) != 0 );
     assert( csd(&T->swapD[index], 0, index) != 0 );
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       atomicMpTerm
//
// Purpose-
//       Verify test results
//
//----------------------------------------------------------------------------
static void
   atomicMpTerm( void )             // Verify test results
{
   TestArea* T= (TestArea*)testPage;
   IFSCDM( snap(T, sizeof(TestArea)); ) // Memory dump

   // Verify simple stores
   for(int index= 0; index<THREAD_COUNT; index++)
   {
//   debugf("index(%2d) P0(%2d) P1(%2d)\n", index,
//          int(T->storePack0[index].d), int(T->storePack1[index].d));

     assert( T->storePack0[index].b == index);
     assert( T->storePack0[index].h == index);
     assert( T->storePack0[index].w == index);
     assert( T->storePack0[index].d == index);

     assert( T->storePack1[index].b == index);
     assert( T->storePack1[index].h == index);
     assert( T->storePack1[index].w == index);
     assert( T->storePack1[index].d == index);

     assert( T->swapPack0[index].b == index);
     assert( T->swapPack0[index].h == index);
     assert( T->swapPack0[index].w == index);
     assert( T->swapPack0[index].d == index);

     assert( T->swapPack1[index].b == index);
     assert( T->swapPack1[index].h == index);
     assert( T->swapPack1[index].w == index);
     assert( T->swapPack1[index].d == index);

     assert( T->swapB[index] == index);
     assert( T->swapH[index] == index);
     assert( T->swapW[index] == index);
     assert( T->swapD[index] == index);
   }

   // Verify completion sequences
   for(int i= 0; i<THREAD_COUNT/8; i++)
   {
     assert( T->rond08[i] == (-1) );
   }

   for(int i= 0; i<THREAD_COUNT/16; i++)
   {
     assert( T->rond16[i] == (-1) );
   }

   for(int i= 0; i<THREAD_COUNT/32; i++)
   {
     assert( T->rond32[i] == (-1) );
   }

   assert( T->rondesvous == (-1) );

   int count;
   int index= T->finalHead->index;
   debugf("Completion:");
   for(count= 0; count<THREAD_COUNT+8; count++)
   {
     debugf(" => %2d", index);
     if( index < 0 )
       break;

     if( (count%10) == 0 )
       debugf("\n  ");
     index= T->sequence[index];
   }
   printf("\n");
// printf("count(%d) index(%d)\n", count, index);
   assert( count == THREAD_COUNT );
   assert( index == (-1) );

   // Completion
   free(testArea);                  // Delete allocated storage
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testAtomicMP
//
// Purpose-
//       Test the Atomic functions in multiprocessor mode
//
//----------------------------------------------------------------------------
static void
   testAtomicMP( void )             // Test Atomic functions on multiprocessor
{
   //-------------------------------------------------------------------------
   // Thread invocation code
   AtomicMpThread**    thread;      // The AtomicMpThread array

   debugf("\n");
   debugf("%s %4d: testAtomicMP()\n", __SOURCE__, __LINE__);

   // Initialize
   testPage= NULL;

   // Allocate the Threads
   thread= (AtomicMpThread**)malloc(THREAD_COUNT * sizeof(AtomicMpThread*));
   for(int i= 0; i<THREAD_COUNT; i++)
     thread[i]= new AtomicMpThread(i);

   // Run the tests
   atomicMpInit();

   for(int i= 0; i<THREAD_COUNT; i++)
     thread[i]->start();

// for(int i= 0; i<THREAD_COUNT; i++)
//   thread[i]->runner();

   // Wait for Threads to complete
   for(int i= 0; i<THREAD_COUNT; i++)
     thread[i]->wait();

   // Test the resultant
   atomicMpTerm();

   // Deallocate Thread array
   for(int i= 0; i<THREAD_COUNT; i++)
     delete thread[i];

   free(thread);

   // Function complete
   debugf("%s %4d: testAtomicMP() complete\n", __SOURCE__, __LINE__);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testAtomicUP
//
// Purpose-
//       Test the Atomic functions in uniprocessor mode.
//
//----------------------------------------------------------------------------
static void
   testAtomicUP( void )             // Test Atomic functions on uniprocessor
{
   ATOMIC8             array8[32];  // Atomic array
   ATOMIC32            array32[32]; // Atomic array

   ATOMIC64            atomic64;
   ATOMIC32            atomic32;
   ATOMIC16            atomic16;
   ATOMIC8             atomic8;
   ATOMICP*            atomicp;

   int                 cc;
   int                 i;
   int                 j;

   debugf("\n");
   debugf("%s %4d: testAtomicUP()\n", __SOURCE__, __LINE__);

   debugf("%s %4d: ..csb()\n", __SOURCE__, __LINE__);
   atomic8= 123;
   cc= csb(&atomic8, 123, 45);
   assert( cc == 0 );
   if( atomic8 != 45 )
     printf("atomic8(%d)\n", atomic8);
   assert( atomic8 == 45 );

   cc= csb(&atomic8, 123, 56);
   assert( cc != 0 );
   assert( atomic8 == 45 );

   debugf("%s %4d: ..csh()\n", __SOURCE__, __LINE__);
   atomic16= 12345;
   cc= csh(&atomic16, 12345, 456);
   assert( cc == 0 );
   assert( atomic16 == 456 );

   cc= csh(&atomic16, 12345, 567);
   assert( cc != 0 );
   assert( atomic16 == 456 );

   debugf("%s %4d: ..csd()\n", __SOURCE__, __LINE__);
   try
   {
     atomic64= 1;
     assert( atomic64 == 1 );
     cc= csd(&atomic64, 1, 2);

     atomic64= 1234567890LL;
     assert( atomic64 == 1234567890LL );
     cc= csd(&atomic64, 1234567890LL, 9876543210LL);

     assert( cc == 0 );
     assert( atomic64 == 9876543210LL );

     cc= csd(&atomic64, 1234567890LL, 0);
     assert( cc != 0 );
     assert( atomic64 == 9876543210LL );
   }
   catch(Exception&e)
   {
     errorf("%s %4d: testAtomicUP Exception(%s)\n", __SOURCE__, __LINE__,
            (const char*)e);
   }
   catch(char* c)
   {
     errorf("%s %4d: testAtomicUP Exception(%s)\n", __SOURCE__, __LINE__, c);
   }
   catch(const char* cc)
   {
     errorf("%s %4d: testAtomicUP Exception(%s)\n", __SOURCE__, __LINE__, cc);
   }
   catch(...)
   {
     errorf("%s %4d: testAtomicUP failure\n", __SOURCE__, __LINE__);
   }

   debugf("%s %4d: ..csp()\n", __SOURCE__, __LINE__);
   atomicp= &atomic32;
   cc= csp( &atomicp, &atomic32, &atomic8);
   assert( cc == 0 );
   assert( atomicp == &atomic8 );

   cc= csp( &atomicp, &atomic32, &atomicp);
   assert( cc != 0 );
   assert( atomicp == &atomic8 );

   debugf("%s %4d: ..csw()\n", __SOURCE__, __LINE__);
   atomic32= 12345678;
   cc= csw(&atomic32, 12345678, 456);
   assert( cc == 0 );
   assert( atomic32 == 456 );

   cc= csw(&atomic32, 12345678, 567);
   assert( cc != 0 );
   assert( atomic32 == 456 );

   debugf("%s %4d: ..tsb()\n", __SOURCE__, __LINE__);
   atomic8= 0;
   cc= tsb( &atomic8 );

   assert( cc == 0 );
   assert( atomic8 == (int8_t)0xff );

   cc= tsb( &atomic8 );
   assert( cc != 0 );
   assert( atomic8 == (int8_t)0xff );

   atomic8= 0x80;
   cc= tsb( &atomic8 );
   assert( cc != 0 );
   assert( atomic8 == (int8_t)0xff );

   for(i= 0; i<32; i++)
   {
     array8[i]= 0;
     array32[i]= 0;
   }

   for(i= 0; i<32; i++)
   {
     cc= csb(&array8[i], 0, i);
     assert( cc == 0 );
     cc= csw(&array32[i], 0, i);
     assert( cc == 0 );
   }

   for(i= 0; i<32; i++)
   {
     assert( array8[i] == i );
     assert( array32[i] == i);
   }

   for(i= 0; i<32; i++)
   {
     cc= csb(&array8[i], 32, i);
     assert( cc != 0 );
     cc= csw(&array32[i], 32, i);
     assert( cc != 0 );
   }

   for(i= 0; i<32; i++)
   {
     assert( array8[i] == i );
     assert( array32[i] == i);
   }

   for(i= 0; i<32; i++)
   {
     cc= tsb(&array8[i]);
     assert( cc == 0 );
   }

   for(i= 0; i<32; i++)
   {
     if( uint8_t(array8[i]) != 0xff ) {
       errorf("array8[%d] == 0x%.2d, not 0xff\n", i, array8[i]);
     }

     assert( uint8_t(array8[i]) == 0xff );
     array8[i]= 0x80;
   }

   for(i= 0; i<32; i++)
   {
     cc= tsb(&array8[i]);
     assert( cc != 0 );
   }

   for(i= 0; i<32; i++)
   {
     assert( uint8_t(array8[i]) == 0xff );
   }

   for(i= 0; i<32; i++)
   {
     array8[i]= 0;
   }

   for(j= 0; j<8; j++)
   {
     for(i= 0; i<32; i++)
     {
       array8[i]= 0;
     }
     cc= csb(&array8[j], 0, j);
     assert( cc == 0 );
     for(i= 0; i<32; i++)
     {
       if( i == j )
         assert( array8[i] == j );
       else
         assert( array8[i] == 0 );
     }
     cc= csb(&array8[j], 32, j);
     assert( cc != 0 );
     for(i= 0; i<32; i++)
     {
       if( i == j )
         assert( array8[i] == j );
       else
         assert( array8[i] == 0 );
     }

     array8[j]= 0x80;
     cc= tsb(&array8[j]);
     assert( cc != 0 );
     for(i= 0; i<32; i++)
     {
       if( i == j )
         assert( uint8_t(array8[i]) == 0xff );
       else
         assert( array8[i] == 0 );
     }
   }

   isync();
   debugf("%s %4d: testAtomicUP() complete\n", __SOURCE__, __LINE__);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testHardware
//
// Purpose-
//       Test the Hardware object functions.
//
//----------------------------------------------------------------------------
static void
   testHardware( void )             // Test Hardware
{
   Hardware            hardware;    // Hardware object

   uint64_t            array[DIM_ARRAY]; // Resultant array
   void*               lr1;         // Return pointer
   void*               lr2;         // Return pointer
   void*               sp1;         // Stack pointer
   void*               sp2;         // Stack pointer

   int                 i;

   debugf("\n");
   debugf("%s %4d: testHardware()\n", __SOURCE__, __LINE__);

   //-------------------------------------------------------------------------
   // Test: Hardware::getLR
   //-------------------------------------------------------------------------
   lr1= Hardware::getLR();          // Try to get different link registers
   lr2= Hardware::getLR();
   IFSCDM(
     debugf("%p= getLR()\n", lr1);
     debugf("%p= getLR()\n", lr2);
   )
   if( lr2 <= lr1 )
   {
     errorCount++;
     debugf("%s %4d: Error: lr2(%p) <= lr1(%p), should be larger\n",
            __SOURCE__, __LINE__, lr1, lr2);
   }

   //-------------------------------------------------------------------------
   // Test: Hardware::getSP
   //-------------------------------------------------------------------------
   sp1= Hardware::getSP();          // Try to get same stack pointer
   sp2= Hardware::getSP();
   IFSCDM(
     debugf("%p= getSP()\n", sp1);
     debugf("%p= getSP()\n", sp2);
   )
   if( sp1 != sp2 )
   {
     errorCount++;
     debugf("%s %4d: Error: sp1(%p) != sp2(%p), should not differ\n",
            __SOURCE__, __LINE__, sp1, sp2);
   }

   sp1= Hardware::getSP();          // Get the Stack pointer
   if( (size_t)sp1 > (size_t)&sp1 )
   {
     errorCount++;
     debugf("%s %4d: Error: sp1(%p) > &sp1(%p)\n", __SOURCE__, __LINE__,
            sp1, &sp1);
   }
   IFSCDM(
     debugf("%s %4d: Stack(%p) Local(%p) Offset(%ld)\n", __SOURCE__, __LINE__,
            sp1, &sp1, (long)((char*)&sp1-(char*)sp1));
   )

   //-------------------------------------------------------------------------
   // Test: Hardware::getTSC()
   //-------------------------------------------------------------------------
   Thread::yield();
   IFSCDM(
     double start= Clock::current();
     while( (Clock::current()-start) < 10.0 )
       debugf("[%6d] 0x%.16" PRIx64 "= Hardware::getTSC()\r",
              0, Hardware::getTSC());
   )

   for(i=0; i<DIM_ARRAY; i++)
   {
     array[i]= Hardware::getTSC();

     IFSCDM(
       debugf("[%6d] 0x%.16" PRIx64 "= Hardware::getTSC()\r", i, array[i]);
     )

//   Thread::yield();
   }
   IFSCDM( debugf("\n"); )

   // Hardware::getTSC diagnostics
   debugf("0x%.16" PRIx64 "= Hardware::getTSC()  (stop)\n", array[DIM_ARRAY-1]);
   debugf("0x%.16" PRIx64 "= Hardware::getTSC() (start)\n", array[0]);
   debugf("%18" PRId64 "= Hardware::getTSC()  (stop)\n", array[DIM_ARRAY-1]);
   debugf("%18" PRId64 "= Hardware::getTSC() (start)\n", array[0]);
   debugf("%18" PRId64 "= cycles\n", array[DIM_ARRAY-1] - array[0]);

   // Each TSC must be greater than the last
   for(i=1; i<DIM_ARRAY; i++)
   {
     if( array[i] <= array[i-1] )
     {
       errorCount++;
       debugf("Hardware.getTSC() increment failure\n");
       debugf("[%6d] %.16" PRIx64 "\n", i-1, array[i-1]);
       debugf("[%6d] %.16" PRIx64 "\n", i, array[i]);
       break;
     }
   }

   IFSCDM(
     for(i=0; i<DIM_ARRAY; i++)
     {
       array[i]= Hardware::getTSC();

       debugf("[%6d] %.16" PRIx64 "= Hardware::getTSC()\r", i, array[i]);
     }
   debugf("\n");
   )
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testSoftware
//
// Purpose-
//       Test the Software object functions.
//
//----------------------------------------------------------------------------
static void
   testSoftware( void )             // Test Software object
{
   unsigned int        id1;         // Process/Thread id
   unsigned int        id2;         // Process/Thread id
   char*               ptrC;        // -> char
   char                string[1024];// Working string

   ptrC= Software::getCwd(string, sizeof(string));
   if( ptrC != string )
   {
     errorCount++;
     printf("Expected(%p), got %p= Software::getCwd()\n", string, ptrC);
   }
   IFSCDM( debugf("%s= Software::getCwd()\n", ptrC); )

   id1= Software::getPid();
   IFSCDM( debugf("%d= Software::getPid()\n", Software::getPid()); )
   id2= Software::getPid();
   if( id1 != id2 )
   {
     errorCount++;
     printf("(%u != %u) Software::getPid() inconsistent\n", id1, id2);
   }

   id1= Software::getTid();
   IFSCDM( debugf("%u= Software::getTid()\n", Software::getTid()); )
   id2= Software::getTid();
   if( id1 != id2 )
   {
     errorCount++;
     printf("(%d != %d) Software::getTid() inconsistent\n", id1, id2);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count
//   char*             argv[])      // Argument array
{
   testAtomicUP();
   testAtomicMP();
   testHardware();
   testSoftware();

   debugf("%s complete, ", __SOURCE__);
   if( errorCount == 0 )
     debugf("NO ");
   else
     debugf("%ld ", errorCount);
   debugf("Error%s\n", errorCount == 1 ? "" : "s");

   return errorCount;
}

