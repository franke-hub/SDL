//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       obj/timing/Latch.h
//
// Purpose-
//       obj/Latch.h timing experiments.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef OBJ_TIMING_LATCH_H_INCLUDED
#define OBJ_TIMING_LATCH_H_INCLUDED

#include <thread>                   // For std::this_thread::yield()

#include "obj/Object.h"             // For std::atomic, Exception
#include "obj/Latch.h"              // The real Latch.h

namespace _OBJ_NAMESPACE {
struct TimingLatch {                // Latch descriptor
//----------------------------------------------------------------------------
// Latch::Attributes
//----------------------------------------------------------------------------
enum
{  RESET= 0                         // Available
,  LOCKED= -1                       // Locked
}; // (generic) enum

volatile unsigned      latch= 0;    // The physical spin latch

//----------------------------------------------------------------------------
// Latch::Methods
//----------------------------------------------------------------------------
void
   lock( void )                     // Obtain the Latch
{
#define USE_FIXED_DELAY 0

#if( USE_FIXED_DELAY > 0 ) // FIXED delay, compare_exchange_strong
//// Currenly way too slow to record. Check out extra Bulk allocations.
   for(;;)
   {
     if( try_lock() ) return;
     if( try_lock() ) return;
     std::this_thread::yield();

     if( try_lock() ) return;
     if( try_lock() ) return;
     std::this_thread::sleep_for(std::chrono::nanoseconds(USE_FIXED_DELAY));
   }
#elif true // Version used in timing tests
   for(unsigned spinCount= 1;;spinCount++)
   {
     if( try_lock() ) return;

     if( (spinCount & 0x0000001f) == 0 )
     {
         unsigned delay= spinCount;
         if( delay > 125000 )
           delay= 125000;

         std::this_thread::sleep_for(std::chrono::microseconds(delay));
     }
   }
#elif true // Current test
     // 0f/10 31.5 @ 70% [w/ browser] 37.9 @ 40% [no browser]
   for(unsigned spinCount= 1;;spinCount++)
   {
     if( try_lock() ) return;

     if( (spinCount & 0x0000000f) == 0 )
     {
       if( (spinCount & 0x00000010) != 0 )
         std::this_thread::yield();
       else
         std::this_thread::sleep_for(std::chrono::nanoseconds(32));
//////// std::this_thread::sleep_for(std::chrono::nanoseconds(spinCount));
//////// Similar result either way.
     }
   }
#else // VARIABLE delay, compare_exchange_strong
   for(unsigned spinCount= 1;;spinCount++)
   {
     if( try_lock() ) return;

     // The default algorithm works OK if chrome is also running, but slows
     // significantly otherwise. It also runs into thread starvation, where
     // one thread gets the Latch much more often than the other threads.
     //
     // This is just an experimental hack with no rationale behind it.

     // 03/04 xx.x @ yy% [w/ browser] 65.8 @ yy% [no browser] [exponential]
     // 03/04 xx.x @ yy% [w/ browser] 63.0 @ yy% [no browser]
     // xx/xx xx.x @ yy% [w/ browser] xx.x @ yy% [no browser]

     // 07/08 30.4 @ 70% [w/ browser] 47.1 @ 25% [no browser] [exponential]
     // 07/08 29.9 @ 70% [w/ browser] 45.2 @ 25% [no browser]
     // 07/0C 30.0 @ 70% [w/ browser] 46.7 @ 25% [no browser]

     // 0f/00 xx.x @ yy% [w/ browser] 59.0 @ 25% [no browser]
     // 0f/10 31.6 @ 70% [w/ browser] 38.9 @ 40% [no browser] *** BEST ***
     // 0f/10 32.3 @ 70% [w/ browser] 38.8 @ 40% [no browser] [exponential]
     // 0f/30 36.5 @ 80% [w/ browser] 38.1 @ 55% [no browser]
     // 0f/70 xx.x @ yy% [w/ browser] 45.0 @ 70% [no browser]

     // 1f/20 33.3 @ 80% [w/ browser] 44.7 @ 40% [no browser] [exponential]
     // 1f/20 35.3 @ 80% [w/ browser] 39.4 @ 50% [no browser]
     // 1f/60 xx.x @ yy% [w/ browser] xx.x @ yy% [no browser]

     // xx/xx xx.x @ yy% [w/ browser] xx.x @ yy% [no browser]
     if( (spinCount & 0x0000001f) == 0 )
     {
       unsigned delay= spinCount;
       if( delay > 125000 )
         delay= 125000;

       std::this_thread::sleep_for(std::chrono::microseconds(delay));
     }

     if( (spinCount & 0x0000000f) == 0 )
     {
       if( (spinCount & 0x00000010) != 0 )
         std::this_thread::yield();
       else
       {
         #if true  /////////////////// NOT exponential
           unsigned delay= spinCount;
         #else
           unsigned delay= spinCount * spinCount;
         #endif
         if( delay > 125000 )
           delay= 125000;

         std::this_thread::sleep_for(std::chrono::nanoseconds(delay));
       }
     }
   }
#endif
}

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the Latch
{  std::atomic<unsigned>* const atomic_latch= (std::atomic<unsigned>*)&latch;

   unsigned oldValue= RESET;
   return atomic_latch->compare_exchange_strong(oldValue, LOCKED);
}

void
   unlock( void )                   // Release the Latch
{  std::atomic<unsigned>* const atomic_latch= (std::atomic<unsigned>*)&latch;

   atomic_latch->store(RESET);
}

void
   reset( void )                    // Initialize/Reset the Latch
{  std::atomic<unsigned>* const atomic_latch= (std::atomic<unsigned>*)&latch;

   atomic_latch->store(RESET);
}
}; // struct TimingLatch

#if false
  #define Latch TimingLatch
#endif

}  // namespace _OBJ_NAMESPACE

#endif // OBJ_TIMING_LATCH_H_INCLUDED
