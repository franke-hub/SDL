//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Interval.h
//
// Purpose-
//       Interval Timer.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef _PUB_INTERVAL_H_INCLUDED
#define _PUB_INTERVAL_H_INCLUDED

#include <chrono>
#include <stdint.h>

#include "config.h"                 // For _PUB_NAMESPACE

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       Interval
//
// Purpose-
//       Interval timer.
//
//----------------------------------------------------------------------------
class Interval {                    // Interval timer
//----------------------------------------------------------------------------
// Interval::Attributes
//----------------------------------------------------------------------------
protected:
std::chrono::steady_clock::time_point
                       start_time;  // Start time
std::chrono::steady_clock::time_point
                       stop_time;   // Stop time

//----------------------------------------------------------------------------
// Interval::Constructors
//----------------------------------------------------------------------------
public:
   ~Interval( void ) {}             // Destructor
   Interval( void )                 // Constructor
:  start_time(),  stop_time()
{  start(); }

//----------------------------------------------------------------------------
// Interval::Operators
//----------------------------------------------------------------------------
   operator double( void ) const    // (double) cast operator
{  return to_double(); }

//----------------------------------------------------------------------------
// Interval::Methods
//----------------------------------------------------------------------------
public:
void
   start( void )                    // Start the Interval Timer
{
   start_time= std::chrono::steady_clock::now();
   stop_time= start_time;
}

double                              // Current time difference (seconds)
   stop( void )                     // Stop the interval timer
{
   stop_time= std::chrono::steady_clock::now();
   return to_double();
}

double                              // Resultant (in seconds)
   to_double( void ) const          // Convert time difference to double
{
   using Nanoseconds= std::chrono::nanoseconds;
   int64_t ns=
       std::chrono::duration_cast<Nanoseconds>(stop_time-start_time).count();
   return (double)ns / 1000000000.0;
}
}; // class Interval
}  // namespace _PUB_NAMESPACE
#endif // _PUB_INTERVAL_H_INCLUDED
