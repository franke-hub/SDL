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
//       Interval.h
//
// Purpose-
//       Interval Timer.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef INTERVAL_H_INCLUDED
#define INTERVAL_H_INCLUDED

#ifndef CLOCK_H_INCLUDED
#include "Clock.h"
#endif

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
Clock                  startTime;   // Start time
Clock                  stopTime;    // Stop time

//----------------------------------------------------------------------------
// Interval::Constructors
//----------------------------------------------------------------------------
public:
   ~Interval( void );               // Destructor
   Interval( void );                // Constructor

   Interval(                        // Constructor
     const Clock&      start,       // Start time
     const Clock&      stop);       // Stop time

//----------------------------------------------------------------------------
// Interval::Operators
//----------------------------------------------------------------------------
   operator double( void ) const;   // (double) cast operator

//----------------------------------------------------------------------------
// Interval::Methods
//----------------------------------------------------------------------------
public:
double                              // Current time difference (0.0)
   start( void );                   // Start the Interval Timer

double                              // Current time difference (seconds)
   stop( void );                    // Stop the interval timer

double                              // Resultant (in seconds)
   toDouble( void ) const;          // Convert time difference to double
}; // class Interval

#endif // INTERVAL_H_INCLUDED
