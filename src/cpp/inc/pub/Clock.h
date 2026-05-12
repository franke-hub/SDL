//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2026 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Clock.h
//
// Purpose-
//       A Clock contains the number of seconds difference from an epoch.
//
// Last change date-
//       2026/04/29
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_CLOCK_H_INCLUDED
#define _LIBPUB_CLOCK_H_INCLUDED

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

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
protected:
double                 second;      // Seconds since Epoch

//----------------------------------------------------------------------------
// Clock::Constructors/destructor
//----------------------------------------------------------------------------
public:
   Clock( void )                    // Default Constructor
:  second(now()) {}                 // (Initializes to current time)

   Clock(                           // Copy Constructor
     const Clock&      clock)       // Source Clock
:  second(clock.second) {}

explicit
   Clock(                           // Constructor
     double            second)      // (The number of seconds since the epoch)
:  second(second) {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   ~Clock( void ) = default;        // Destructor

//----------------------------------------------------------------------------
// Clock::Operators
//----------------------------------------------------------------------------
// Cast operators- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   explicit operator double( void ) const // Cast to double
{  return second; }                 // (Seconds since epoch)

// Assignment operators- - - - - - - - - - - - - - - - - - - - - - - - - - - -
Clock&                              // Resultant
   operator=(                       // Assignment operator
     const Clock&      clock)       // Source Clock
{  second= clock.second; return *this; }

Clock&                              // Resultant
   operator=(                       // Assignment operator
     double            second)      // Source (seconds since epoch)
{  this->second= second; return *this; }

// Arithmetic operators- - - - - - - - - - - - - - - - - - - - - - - - - - - -
Clock&                              // Resultant
   operator+=(                      // Add to this
     const Clock&      rhs)         // Addend
{  second += rhs.second; return *this; }

Clock&                              // Resultant
   operator+=(                      // Add to this
     const double      rhs)         // Addend
{  second += rhs; return *this; }

Clock&                              // Resultant
   operator-=(                      // Subtract from this
     const Clock&      rhs)         // Subtrahend
{  second -= rhs.second; return *this; }

Clock&                              // Resultant
   operator-=(                      // Add to this
     const double      rhs)         // Addend
{  second -= rhs; return *this; }

friend Clock                        // Resultant
   operator+(                       // (Global) Add to
     const Clock&      lhs,         // Augend
     const Clock&      rhs)         // Addend
{  Clock out(lhs); out += rhs; return out; }

friend Clock                        // Resultant
   operator+(                       // (Global) Add to
     const Clock&      lhs,         // Augend
     const double      rhs)         // Addend
{  Clock out(lhs); out += rhs; return out; }

friend Clock                        // Resultant
   operator-(                       // (Global) Subtract from
     const Clock&      lhs,         // Minuend
     const Clock&      rhs)         // Subtrahend
{  Clock out(lhs); out -= rhs; return out; }

friend Clock                        // Resultant
   operator-(                       // (Global) Add to
     const Clock&      lhs,         // Augend
     const double      rhs)         // Addend
{  Clock out(lhs); out -= rhs; return out; }

// Comparison operators- - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // Resultant
   operator==(                      // Compare to
     const Clock&      rhs) const   // Comprahend
{  return second == rhs.second; }

bool                                // Resultant
   operator!=(                      // Compare to
     const Clock&      rhs) const   // Comprahend
{  return second != rhs.second; }

bool                                // Resultant
   operator<(                       // Compare to
     const Clock&      rhs) const   // Comprahend
{  return second < rhs.second; }

bool                                // Resultant
   operator<=(                      // Compare to
     const Clock&      rhs) const   // Comprahend
{  return second <= rhs.second; }

bool                                // Resultant
   operator>(                       // Compare to
     const Clock&      rhs) const   // Comprahend
{  return second > rhs.second; }

bool                                // Resultant
   operator>=(                      // Compare to
     const Clock&      rhs) const   // Comprahend
{  return second >= rhs.second; }

//----------------------------------------------------------------------------
// Clock::Accessor methods
//----------------------------------------------------------------------------
double                              // The number of seconds since the epoch
   get( void ) const                // Get number of seconds since the epoch
{  return second; }

void
   set(                             // Set the Clock
     double            second)      // The number of seconds since the epoch
{  this->second= second; }

//----------------------------------------------------------------------------
// Clock::Methods
//----------------------------------------------------------------------------
static double                       // The number of seconds since the epoch
   now( void );                     // Get number of seconds since the epoch
};
_LIBPUB_END_NAMESPACE
#endif  // _LIBPUB_CLOCK_H_INCLUDED
