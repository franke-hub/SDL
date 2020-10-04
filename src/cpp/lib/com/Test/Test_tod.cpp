//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_tod.cpp
//
// Purpose-
//       Test time functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>

#include "com/Calendar.h"
#include "com/Clock.h"
#include "com/Interval.h"
#include "com/Julian.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
//atic const double    EPSILON= 1.0e-6; // A small time value
#define NANO_PER_SECOND 1000000000  // Nanoseconds per second
#define TIME_INTERVAL 4.25          // Time interval, in seconds

//----------------------------------------------------------------------------
//
// Subroutine-
//       getSecond
//
// Purpose-
//       Return a clock second.
//
//----------------------------------------------------------------------------
static int64_t                      // The clock second
   getSecond(                       // Get clock second
     const Clock&      clock)       // For this Clock
{
   double time= clock.getTime();
   int64_t second= (int64_t)time;   // Get current second
   if( time < double(second) )      // Time must be >= second
     --second;

   return second;
}

static int64_t                      // The Julian second
   getSecond(                       // Get Julian second
     const Julian&     julian)      // For this Julian
{
   double time= julian.getTime();
   int64_t second= (int64_t)time;   // Get current second
   if( time < double(second) )      // Time must be >= second
     --second;

   return second;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       getNanosecond
//
// Purpose-
//       Return a clock nanosecond.
//
//----------------------------------------------------------------------------
static unsigned long                // The clock nanosecond
   getNanosecond(                   // Get clock nanosecond
     const Clock&      clock)       // For this Clock
{
   int64_t second= getSecond(clock); // Get current second
   return (unsigned long)((clock.getTime() - double(second)) * NANO_PER_SECOND);
}

static unsigned long                // The Julian nanosecond
   getNanosecond(                   // Get Julian nanosecond
     const Julian&     julian)      // For this Julian
{
   int64_t second= getSecond(julian); // Get current second
   return (unsigned long)((julian.getTime() - double(second)) * NANO_PER_SECOND);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       showClock
//
// Purpose-
//       Having issues with granule, displaying associated Clock.
//
//----------------------------------------------------------------------------
static inline void
   showClock(                       // Display clocks
     int               line,        // Line number
     const Clock&      L,           // Left  hand clock
     const Clock&      R)           // Right hand clock
{
#if FALSE
   tracef("%4d {%24.18e %24.18e} %s\n", line, L.getTime(), R.getTime(),
          (L.getTime()-R.getTime() < 3.0e-7) ? "==" : "!=");
#elif FALSE
   tracef("%4d {%24.18e %24.18e} %s\n", line, L.getTime(), R.getTime(),
          (L.getTime() == R.getTime()) ? "==" : "!=");
#else                               // Parameters unused
   (void)line;
   (void)L;
   (void)R;
#endif
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
   Julian              delJ, oldJ, nowJ, usrJ;
   Clock               delC, oldC, nowC, usrC;
   Interval            interval;
   long                counter;

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   usrC= TIME_INTERVAL;             // Delay interval
   usrJ= TIME_INTERVAL;             // Delay interval

   //-------------------------------------------------------------------------
   // Compute granule
   //-------------------------------------------------------------------------
   #define USE_IFGRANULE            // Alternate implementation
   double EPSILON= nowC * 2.25e-16; // A relatively small interval

   double granule;
   double smallest= 9999.9;         // For interesting math effect
   counter= 0;
   nowC= Clock::current();
   for(;;)                          // Wait for the time to change
   {
     oldC= Clock::current();
     #ifdef USE_IFGRANULE
       showClock(__LINE__, oldC, nowC);
       granule= oldC.getTime() - nowC.getTime();
       if( granule > EPSILON )
         break;

       if( granule != 0.0 && granule < smallest )
         smallest= granule;
     #else
       showClock(__LINE__, oldC, nowC);
       if( (oldC.getTime() - nowC.getTime()) > EPSILON )
         break;
     #endif
   }

   for(;;)                          // Wait for the time to change again
   {
     nowC= Clock::current();
     #ifdef USE_IFGRANULE
       showClock(__LINE__, nowC, oldC);
       granule= nowC.getTime() - oldC.getTime();
       if( granule > EPSILON )
         break;

       if( granule != 0.0 && granule < smallest )
         smallest= granule;
     #else
       showClock(__LINE__, nowC, oldC);
       if( (nowC.getTime() - oldC.getTime()) > EPSILON )
         break;
     #endif
     counter++;
   }

   showClock(__LINE__, nowC, oldC);

#if 0 // See GNU bugzilla 323 to see why values both statements could be true
      // (If EPSILON is sufficiently near zero.)
   if( nowC.getTime() != oldC.getTime() )
     debugf("Difference\n");

   if( (nowC.getTime() - oldC.getTime()) == 0.0 )
     debugf("No difference\n");
#endif

   #ifndef USE_GRANULE
     granule= nowC.getTime() - oldC.getTime(); // The time granule
   #endif
   debugf(" Granule: %.18f seconds\n", granule);
   debugf(" Counter: %ld\n", counter);
   debugf("     New: %24.18e seconds\n", (double)nowC);
   debugf("     Old: %24.18e seconds\n", (double)oldC);
   debugf(" EPSILON: %24.18e seconds\n", EPSILON);
   debugf(" Bug 323: %24.18e seconds\n", smallest);

#if FALSE // DEMO bugzilla 323 strangeness (occurs only in GCC)
   counter= 0;
   oldC= Clock::current();
   nowC= Clock::current();
   while( nowC >= oldC )
   {
     oldC= Clock::current();
     nowC= Clock::current();
     counter++;
   }
   debugf("Bugzilla 323 demo complete, %ld retries\n", counter);
#endif

   if( FALSE )                      // For quick exit in bringup
     return 0;

   //-------------------------------------------------------------------------
   // Clock measurement
   //-------------------------------------------------------------------------
   oldC= Clock::current();
   debugf("\n\n");
   debugf("----Clock Start Time\n");
   debugf("%20.9f\n", (double)oldC);

   debugf("------------Current Time  ");
   debugf("--------------Difference\n");
   for(;;)
   {
     nowC= Clock::current();
     delC= nowC - oldC;
     debugf("%24.9f  %24.9f\r", (double)nowC, (double)delC);
     if( delC >= usrC )
       break;
   }
   debugf("\n\n");
   debugf("----------------Interval\n" "%14lu.%.9lu seconds\n",
          (long)getSecond(delC), (long)getNanosecond(delC));

   //-------------------------------------------------------------------------
   // Julian measurement
   //-------------------------------------------------------------------------
   oldJ= Julian::current();
   debugf("\n\n");
   debugf("-------Julian Start Time\n");
   debugf("%24.9f\n", (double)oldJ);

   debugf("------------Current Time  ");
   debugf("--------------Difference\n");
   for(;;)
   {
     nowJ= Julian::current();
     delJ= nowJ - oldJ;
     debugf("%24.9f  %24.9f\r", (double)nowJ, (double)delJ);
     if( delJ >= usrJ )
       break;
   }
   debugf("\n\n");
   debugf("----------------Interval\n" "%14lu.%.9lu seconds\n",
          long(getSecond(delJ)), long(getNanosecond(delJ)));

   //-------------------------------------------------------------------------
   // Interval measurement
   //-------------------------------------------------------------------------
   debugf("\n\n");
   debugf("-----Interval Start Time\n");
   debugf("%24.9f\n", Clock::current());
   debugf("------------Current Time  ");
   debugf("----------------Interval\n");
   interval.start();
   for(;;)
   {
     interval.stop();
     debugf("%24.9f  %24.9f\r", Clock::current(), (double)(interval));
     if( interval.toDouble() >= TIME_INTERVAL )
       break;
   }
   debugf("\n\n");

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   return 0;
}

