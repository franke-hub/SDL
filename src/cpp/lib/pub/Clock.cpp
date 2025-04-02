//----------------------------------------------------------------------------
//
//       Copyright (C) 2007-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
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
//       2025/03/30
//
//----------------------------------------------------------------------------
#include <ctime>                    // For clock_gettime

#include <sys/timeb.h>              // For struct timeb

#include "pub/Clock.h"              // For pub::Clock, implemented

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Method-
//       Clock::now
//
// Purpose-
//       Return the number of seconds since the PC epoch.
//
//----------------------------------------------------------------------------
double                              // Seconds since the PC epoch
   Clock::now( void )               // Get the current time
{
   struct timespec     ticker;      // UTC time base

   clock_gettime(CLOCK_REALTIME, &ticker); //
   double seconds= (double)ticker.tv_sec;
   seconds += (double)ticker.tv_nsec / 1000000000.0;
   return seconds;
}
} // namespace _LIBPUB_NAMESPACE
