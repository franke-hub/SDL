//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2014 Frank Eskesen.
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
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifdef _OS_WIN
  #include <windows.h>
  #include <sys/timeb.h>
#else
  #include <time.h>
  #include <sys/timeb.h>
#endif
#include <stdint.h>

#include <com/Debug.h>
#include <com/Julian.h>

#include "com/Clock.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Clock::Clock(const Julian&)
//
// Purpose-
//       Copy constructor.
//
//----------------------------------------------------------------------------
   Clock::Clock(                    // Copy constructor
     const Julian&     source)      // Source Julian
{
   time= source.getTime() - Julian::getUTC1970Time();
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::operator=(const Julian&)
//
// Purpose-
//       Assignment operator
//
//----------------------------------------------------------------------------
Clock&                              // Resultant
   Clock::operator=(                // Assignment operator
     const Julian&     source)      // Source Julian
{
   time= source.getTime() - Julian::getUTC1970Time();
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::current
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
#if FALSE
double                              // The time of day
   Clock::current( void )           // Get the the current time
{
   double              result;      // Resultant time
   struct timeb        ticker;      // UTC time base

   ftime(&ticker);                  // UTC (since epoch)
   result  = (double)ticker.time;
   result += (double)ticker.millitm / 1000.0;

   return result;
}

#elif defined(_OS_WIN)
   // This should work more accurately, but doesn't. Precision is 0.016 seconds
   #define FILETIMES_PER_SECOND 10000000.0 // FILETIME intervals/second

   // The number of seconds between the FILETIME epoch and the CLOCK epoch
   static const double FILETIME_TO_CLOCK=
                           (2440588.0-2305814.0) * Julian::SECONDS_PER_DAY;

double                              // The time of day
   Clock::current( void )           // Get the the current time
{
   FILETIME            time;        // Resultant FILETIME

// GetSystemTimePreciseAsFileTime(&time); // Get FILETIME (Undefined at RUNTIME)
   GetSystemTimeAsFileTime(&time);  // Get FILETIME

   double result= (uint64_t(time.dwHighDateTime) << 32) | uint64_t(time.dwLowDateTime);
   result /= FILETIMES_PER_SECOND;  // Normalize to seconds
   result -= FILETIME_TO_CLOCK;     // Convert to Clock epoch
   return result;
}

#else
double                              // The time of day
   Clock::current( void )           // Get the the current time
{
   double              result;      // Resultant time
   struct timespec     ticker;      // UTC time base

   clock_gettime(CLOCK_REALTIME, &ticker); //
   result  = (double)ticker.tv_sec;
   result += (double)ticker.tv_nsec / 1000000000.0;
   return result;
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Clock::toJulian
//
// Purpose-
//       Convert to Julian
//
//----------------------------------------------------------------------------
Julian                              // The Julian
   Clock::toJulian( void ) const    // Convert to Julian
{
   Julian result(*this);
   return result;
}

