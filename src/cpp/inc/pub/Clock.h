//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Clock.h
//
// Purpose-
//       A Clock contains a positive time offset from an Epoch.
//
// Last change date-
//       2022/09/02
//
// Implementation notes-
//       An Epoch is an arbitrary time origin which cannot change without
//       a machine reboot. The current Epoch began Jan 1, 1970 and provides
//       for at least microsecond clock resolution until the year 2100.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_CLOCK_H_INCLUDED
#define _LIBPUB_CLOCK_H_INCLUDED

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       Clock
//
// Purpose-
//       Define the local clock.
//
//----------------------------------------------------------------------------
class Clock {                       // Local clock
//----------------------------------------------------------------------------
// Clock::Attributes
//----------------------------------------------------------------------------
private:
double                 time;        // Seconds since Epoch

//----------------------------------------------------------------------------
// Clock::Constructors
//----------------------------------------------------------------------------
public:
   ~Clock( void ) {}                // Destructor

   Clock( void )                    // Default Constructor (Current time)
:  time(now()) {}

   Clock(                           // Copy Constructor
     const Clock&      source)      // Source
:  time(source.time) {}

   Clock(                           // Constructor
     double            source)      // Source (seconds since Epoch)
:  time(source) {}

Clock&                              // Resultant
   operator=(                       // Assignment operator
     const Clock&      source)      // Source
{
   time= source.time;
   return *this;
}

Clock&                              // Resultant
   operator=(                       // Assignment operator
     double            source)      // Source (seconds since Epoch)
{
   time= source;
   return *this;
}

//----------------------------------------------------------------------------
// Clock::Operators
//----------------------------------------------------------------------------
public:
Clock&                              // Resultant
   operator+=(                      // Add to this
     const Clock&      rhs)         // Addend
{  time += rhs.time;
   return *this;
}

friend Clock                        // Resultant
   operator+(                       // (Global) Add to
     const Clock&      lhs,         // Augend
     const Clock&      rhs)         // Addend
{  Clock sum(lhs);
   sum += rhs;
   return sum;
}

Clock&                              // Resultant
   operator-=(                      // Subtract from this
     const Clock&      rhs)         // Subtrahend
{  time -= rhs.time;
   return *this;
}

friend Clock                        // Resultant
   operator-(                       // (Global) Subtract from
     const Clock&      lhs,         // Minuend
     const Clock&      rhs)         // Subtrahend
{  Clock diff(lhs);
   diff -= rhs;
   return diff;
}

bool                                // Resultant
   operator!=(                      // Compare to
     const Clock&      rhs) const   // Comprahend
{  return time != rhs.time; }

bool                                // Resultant
   operator<=(                      // Compare to
     const Clock&      rhs) const   // Comprahend
{  return time <= rhs.time; }

bool                                // Resultant
   operator==(                      // Compare to
     const Clock&      rhs) const   // Comprahend
{  return time == rhs.time; }

bool                                // Resultant
   operator>=(                      // Compare to
     const Clock&      rhs) const   // Comprahend
{  return time >= rhs.time; }

bool                                // Resultant
   operator<(                       // Compare to
     const Clock&      rhs) const   // Comprahend
{  return time < rhs.time; }

bool                                // Resultant
   operator>(                       // Compare to
     const Clock&      rhs) const   // Comprahend
{  return time > rhs.time; }

   explicit operator double( void ) const // Cast to double
{  return time; }

//----------------------------------------------------------------------------
// Clock::Accessors
//----------------------------------------------------------------------------
public:
double                              // The seconds since epoch
   get( void ) const                // Get seconds since epoch
{  return time; }

void
   set(                             // Set the Clock
     double            time)        // Time, seconds sincd epoch
{  this->time= time; }

//----------------------------------------------------------------------------
// Clock::Methods
//----------------------------------------------------------------------------
public:
static double                       // The number of seconds since the Epoch
   now( void );                     // Get number of seconds since the Epoch
};
_LIBPUB_END_NAMESPACE
#endif  // _LIBPUB_CLOCK_H_INCLUDED
