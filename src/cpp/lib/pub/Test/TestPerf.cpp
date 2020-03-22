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
//       TestPerf.cpp
//
// Purpose-
//       Performance test (for machine to machine comparison)
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <locale.h>                 // For setlocale

#include <pub/Debug.h>
#include <pub/Interval.h>

using namespace _PUB_NAMESPACE;
using namespace _PUB_NAMESPACE::debugging;

//----------------------------------------------------------------------------
// Internal data areas. Counters external to confuse compiler
//----------------------------------------------------------------------------
volatile unsigned int  int_counter= 0;
volatile unsigned long long_counter= 0;
volatile double        dbl_counter= 0.0;

//----------------------------------------------------------------------------
//
// Subroutine-
//       testtime
//
// Purpose-
//       Timing tests.
//
//----------------------------------------------------------------------------
static inline void
   testtime( void )                 // Timing tests
{
   Interval            interval;    // Interval timer

   interval.start();
   while( interval.stop() < 1.0 )
     int_counter++;

   interval.start();
   while( interval.stop() < 1.0 )
     long_counter++;

   interval.start();
   while( interval.stop() < 1.0 )
     dbl_counter += 1.0;

   // Test complete
   debugf("%'16d ints/second\n", int_counter);
   debugf("%'16ld longs/second\n", long_counter);
   debugf("%'16.0f doubles/second\n", dbl_counter);
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
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   setlocale(LC_ALL, "");           // Activates ' thousand separator
   testtime();

   return 0;
}
