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
//       Julian.cpp
//
// Purpose-
//       Implement Julian.h
//
// Last change date-
//       2025/03/30
//
//----------------------------------------------------------------------------
#include <exception>                // For std::range_error
#include <math.h>                   // For floor, fmod
#include <stdint.h>                 // For INT64_MAX, INT64_MIN

#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/Clock.h"              // For pub::Clock
#include "pub/Julian.h"             // For pub::Julian, implemented

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging methods
using pub::Clock;                   // For convenience

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // generic enum

//----------------------------------------------------------------------------
// Static constants
//----------------------------------------------------------------------------
const Julian           Julian::DAYZERO(        0.0);
const Julian           Julian::UTC0001(1'721'424.0);
const Julian           Julian::UTC1600(2'305'448.0);
const Julian           Julian::UTC1900(2'415'021.0);
const Julian           Julian::UTC1970(2'440'588.0);
const Julian           Julian::UTC2000(2'451'545.0);

//----------------------------------------------------------------------------
//
// Method-
//       Julian::operator Clock
//
// Purpose-
//       Cast to Clock
//
//----------------------------------------------------------------------------
   Julian::operator Clock( void ) const // Cast to Clock
{  Clock clock((day - (double)UTC1970) * SECONDS_PER_DAY); return clock; }

//----------------------------------------------------------------------------
//
// Method-
//       Julian::get_tod
//
// Purpose-
//       Get the time of day, range 0 .. <(SECONDS_PER_DAY)
//
//----------------------------------------------------------------------------
double                              // The time of day
   Julian::get_tod( void ) const    // Get time of day
{  return (day - floor(day)) * SECONDS_PER_DAY; }

//----------------------------------------------------------------------------
//
// Method-
//       Julian::now
//
// Purpose-
//       Get the current Julian second
//
//----------------------------------------------------------------------------
double                              // The current Julian second
   Julian::now( void )              // Get current Julian second
{  return (double)UTC1970 + Clock::now() / (double)SECONDS_PER_DAY; }

//----------------------------------------------------------------------------
//
// Method-
//       Julian::set(const Clock&)
//
// Purpose-
//       Set the Julian day from a Clock
//
//----------------------------------------------------------------------------
void
   Julian::set(                     // Set the Julian
     const Clock&      clock)       // From this Clock
{  day= ((double)clock / SECONDS_PER_DAY) + (double)UTC1970; }
} // namespace _LIBPUB_NAMESPACE
