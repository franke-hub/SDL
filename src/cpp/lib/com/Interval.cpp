//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Interval.cpp
//
// Purpose-
//       Interval Timer.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#include "com/Interval.h"

//----------------------------------------------------------------------------
//
// Method-
//       Interval::~Interval
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Interval::~Interval( void )      // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Interval::Interval
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Interval::Interval( void )       // Constructor
:  startTime()
,  stopTime()
{
   start();
}

   Interval::Interval(              // Constructor
     const Clock&      start,       // Start time
     const Clock&      stop)        // Stop  time
:  startTime(start)
,  stopTime(stop)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Interval::operator double
//
// Purpose-
//       Cast to double
//
//----------------------------------------------------------------------------
   Interval::operator double( void ) const // Cast to double
{
   return stopTime.getTime() - startTime.getTime();
}

//----------------------------------------------------------------------------
//
// Method-
//       Interval::toDouble
//
// Purpose-
//       Cast to double.
//
//----------------------------------------------------------------------------
double                              // Time difference
   Interval::toDouble( void ) const // Cast to double
{
   return operator double();
}

//----------------------------------------------------------------------------
//
// Method-
//       Interval::start
//
// Purpose-
//       Start the interval timer.
//
//----------------------------------------------------------------------------
double                              // Current time difference
   Interval::start( void )          // Start the Interval Timer
{
   stopTime= startTime= Clock::current();
   return 0.0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Interval::stop
//
// Purpose-
//       Stop the interval timer.
//
//----------------------------------------------------------------------------
double                              // Current time difference
   Interval::stop( void )           // Stop the interval timer
{
   stopTime= Clock::current();
   return operator double();
}

