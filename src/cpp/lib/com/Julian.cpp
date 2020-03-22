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
//       Julian.cpp
//
// Purpose-
//       Julian object methods.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#include <com/Clock.h>
#include <com/Debug.h>

#include "com/Julian.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Static constants
//----------------------------------------------------------------------------
const Julian           Julian::UTC0001(1721424.0 * Julian::SECONDS_PER_DAY);
const Julian           Julian::UTC1601(2305814.0 * Julian::SECONDS_PER_DAY);
const Julian           Julian::UTC1900(2415021.0 * Julian::SECONDS_PER_DAY);
const Julian           Julian::UTC1970(2440588.0 * Julian::SECONDS_PER_DAY);
const Julian           Julian::UTC2000(2451545.0 * Julian::SECONDS_PER_DAY);

//----------------------------------------------------------------------------
//
// Method-
//       Julian::Julian(const Clock&)
//
// Function-
//       Copy constructor.
//
//----------------------------------------------------------------------------
   Julian::Julian(                  // Copy constructor
     const Clock&      source)      // Source
{
   time= source.getTime() + UTC1970.getTime();
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::operator=
//
// Function-
//       Assign to this Julian.
//
//----------------------------------------------------------------------------
Julian&                             // Resultant
   Julian::operator=(               // Assign to this
     const Clock&      source)      // Source
{
   time= source.getTime() + UTC1970.getTime();
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::current
//
// Purpose-
//       Get the current Julian second
//
//----------------------------------------------------------------------------
double                              // The current Julian second
   Julian::current( void )          // Get current Julian second
{
   return Clock::current() + UTC1970.getTime();
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::getUTC1601Time
//
// Function-
//       Return the UTC1601 Julian second.
//
//----------------------------------------------------------------------------
double                              // The UTC1601 Julian second
   Julian::getUTC1601Time( void )   // Get UTC1601 Julian second
{
   return UTC1601.time;
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::getUTC1970Time
//
// Function-
//       Return the UTC1970 Julian second.
//
//----------------------------------------------------------------------------
double                              // The UTC1970 Julian second
   Julian::getUTC1970Time( void )   // Get UTC1970 Julian second
{
   return UTC1970.time;
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::toClock
//
// Purpose-
//       Convert to Clock
//
//----------------------------------------------------------------------------
Clock                               // The Clock
   Julian::toClock( void ) const    // Convert to Clock
{
   Clock result(*this);
   return result;
}

