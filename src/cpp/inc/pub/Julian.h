//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2025 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Julian.h
//
// Purpose-
//       The Julian object represents a MODIFIED Julian date and time.
//
// Last change date-
//       2025/03/21
//
// Implementation Notes-
//       This MODIFIED Julian date begins at midnight rather than noon.
//       (Add 0.5 to this modified value to get the associated astronomical
//       Julian day value.)
//
//       The first modified Julian day began at midnight on
//       Monday, January 1st, 4713 BCE (using the proleptic Julian calendar)
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_JULIAN_H_INCLUDED
#define _LIBPUB_JULIAN_H_INCLUDED

#include "pub/Clock.h"              // For pub::Clock

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       Julian
//
// Purpose-
//       The Julian object.
//
//----------------------------------------------------------------------------
class Julian {                      // Julian Date
//----------------------------------------------------------------------------
// Julian::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum                                // Conversion factors
{  SECONDS_PER_DAY=   86'400        // Seconds per day
,  SECONDS_PER_HOUR=   3'600        // Seconds per hour
,  SECONDS_PER_MINUTE=    60        // Seconds per minute
,  MINUTES_PER_HOUR=      60        // Minutes per second
,  HOURS_PER_DAY=         24        // Hours per day
,  MONTHS_PER_YEAR=       12        // Months per year
}; // enum Conversion factors

//----------------------------------------------------------------------------
// Julian::Attributes
//----------------------------------------------------------------------------
protected:
double                 day;         // Days since epoch

//----------------------------------------------------------------------------
// Julian::Constants
//----------------------------------------------------------------------------
public:
static const Julian    DAYZERO;     // Jan 1, -4712 (Proleptic Julian day zero)
static const Julian    UTC0001;     // Jan 1,  0001 (Julian calendar)
static const Julian    UTC1600;     // Jan 1,  1600 (Gregorian calendar)
static const Julian    UTC1900;     // Jan 1,  1900
static const Julian    UTC1970;     // Jan 1,  1970 (PC Epoch)
static const Julian    UTC2000;     // Jan 1,  2000

//----------------------------------------------------------------------------
// Julian::Constructors/destructor
//----------------------------------------------------------------------------
   Julian( void )                   // Default Constructor (current day)
:  day(now()) {}                    // (Initializes to current fractional day)

   Julian(                          // Copy Constructor
     const Julian&     julian)      // Source Julian
:  day(julian.day) {}

   Julian(                          // Constructor
     double            day)         // (Days since epoch)
:  day(day) {}

   Julian(                          // Copy Constructor
     const Clock&      clock)       // Source Clock
{  set(clock); }

   ~Julian( void ) = default;       // Destructor

//----------------------------------------------------------------------------
// Julian::Operators
//----------------------------------------------------------------------------
// Cast operators- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   explicit operator double( void ) const // Cast to double
{  return day; }                    // (Days since epoch)

   explicit operator Clock( void ) const; // Cast to Clock

// Assignment operators- - - - - - - - - - - - - - - - - - - - - - - - - - - -
Julian&                             // Resultant
   operator=(                       // Assignment operator
     double            day)         // Source (Julian) day
{  set(day); return *this; }

Julian&                             // Resultant
   operator=(                       // Assignment operator
     const Clock&      clock)       // Source Clock
{  set(clock); return *this; }

Julian&                             // Resultant
   operator=(                       // Assignment operator
     const Julian&     julian)      // Source Julian
{  set(julian); return *this; }

// Arithmetic operators- - - - - - - - - - - - - - - - - - - - - - - - - - - -
Julian&                             // Resultant
   operator++( void )               // (++operator)
{  day += 1.0; return *this; }

Julian                              // Resultant
   operator++( int )                // (operator++)
{  Julian julian(*this); day += 1.0; return julian; }

Julian&                             // Resultant
   operator--( void )               // (--operator)
{  day -= 1.0; return *this; }

Julian                              // Resultant
   operator--( int )                // (operator--)
{  Julian julian(*this); day -= 1.0; return julian; }

Julian&                             // Resultant
   operator+=(                      // Add to this
     const Julian&     rhs)         // Addend
{  day += rhs.day; return *this; }

Julian&                             // Resultant
   operator-=(                      // Subtract from this
     const Julian&     rhs)         // Subtrahend
{  day -= rhs.day; return *this; }

friend Julian                       // Resultant
   operator+(                       // Add to
     const Julian&     lhs,         // Augend
     const Julian&     rhs)         // Addend
{  Julian out(lhs); out += rhs; return out; }

friend Julian                       // Resultant
   operator-(                       // Subtract from
     const Julian&     lhs,         // Minuend
     const Julian&     rhs)         // Subtrahend
{  Julian out(lhs); out -= rhs; return out; }

// Comparison operators- - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Equality comparisons are truncated to the microsecond
bool                                // Resultant
   operator==(                      // Compare to
     const Julian&     rhs) const   // Comprahend
{  return int64_t(day*1000000.0) == int64_t(rhs.day*1000000.0); }

bool                                // Resultant
   operator!=(                      // Compare to
     const Julian&     rhs) const   // Comprahend
{  return int64_t(day*1000000.0) != int64_t(rhs.day*1000000.0); }

bool                                // Resultant
   operator<(                       // Compare to
     const Julian&     rhs) const   // Comprahend
{  return day < rhs.day; }

bool                                // Resultant
   operator<=(                      // Compare to
     const Julian&     rhs) const   // Comprahend
{  return day <= rhs.day; }

bool                                // Resultant
   operator>(                       // Compare to
     const Julian&     rhs) const   // Comprahend
{  return day > rhs.day; }

bool                                // Resultant
   operator>=(                      // Compare to
     const Julian&     rhs) const   // Comprahend
{  return day >= rhs.day; }

//----------------------------------------------------------------------------
// Julian::Accessor methods
//----------------------------------------------------------------------------
double                              // The Julian (fractional) day
   get( void ) const                // Get Julian (fractional) day
{  return day; }

double                              // The Julian (fractional) time of day
   get_tod( void ) const;           // Get Julian (fractional) time of day

void
   set(                             // Set the Julian
     double            day)         // The Julian (fractional) day
{  this->day= day; }

//----------------------------------------------------------------------------
// Julian::Methods
//----------------------------------------------------------------------------
static double                       // The Julian (fractional) day
   now( void );                     // Get Julian (fractional) day

void
   set( void )                      // Set the Julian to now()
{  day= now(); }

void
   set(                             // Set the Clock
     const Clock&      clock);      // From this Clock

void
   set(                             // Set the Clock
     const Julian&     julian)      // From this Julian
{  day= julian.day; }
}; // class Julian
_LIBPUB_END_NAMESPACE
#endif  // _LIBPUB_JULIAN_H_INCLUDED
