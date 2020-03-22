//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2018 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Clock.cpp
//
// Purpose-
//       Clock object methods.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#define USE_EXPERIMENTAL false      // Note: No difference in result
#if USE_EXPERIMENTAL
#include <chrono>
#include <stdint.h>

#else
#ifdef _OS_WIN
  #include <windows.h>
  #include <sys/timeb.h>
#else
  #include <time.h>
  #include <sys/timeb.h>
#endif
#endif // USE_EXPERIMENTAL

#include "pub/Clock.h"

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Method-
//       Clock::now
//
// Purpose-
//       Return the current time of day. (Seconds past PC Epoch.)
//
// Implementation notes-
//       On Windows systems, the best resolution that can be obtained is
//       about 1/64 second. The call GetSystemTimePreciseAsFileTime links
//       but fails when the function is actually called.
//
//----------------------------------------------------------------------------
double                              // The time of day
   Clock::now( void )               // Get the the current time
{
#if USE_EXPERIMENTAL
   using Clock = std::chrono::system_clock;
   const Clock::duration delta= Clock::now().time_since_epoch();
   using Ns= std::chrono::nanoseconds;
   int64_t ns= std::chrono::duration_cast<Ns>(delta).count();
   return (double)ns / 1000000000.0;
#else
   double              result;      // Resultant time
   struct timespec     ticker;      // UTC time base

   clock_gettime(CLOCK_REALTIME, &ticker); //
   result  = (double)ticker.tv_sec;
   result += (double)ticker.tv_nsec / 1000000000.0;
   return result;
#endif
}
}  // namespace _PUB_NAMESPACE
