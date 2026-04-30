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
//       Calendar.h
//
// Purpose-
//       The Calendar object contains date and time information.
//
// Last change date-
//       2026/04/29
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_CALENDAR_H_INCLUDED
#define _LIBPUB_CALENDAR_H_INCLUDED

#include <string>                   // For std::string
#include <cstdint>                  // For integer types: int64_t, ...

#include "pub/Clock.h"              // For pub::Clock
#include "pub/Julian.h"             // For pub::Julian

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       Calendar
//
// Purpose-
//       The Calendar object.
//
//----------------------------------------------------------------------------
class Calendar {                    // Calendar
//----------------------------------------------------------------------------
// Calendar::Typedefs and enumerations
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

// The conversion date: Julian to Gregorian Calendar
enum SWITCH_                        // The conversion date
{  SWITCH_MM= 10                    // The Calendar month
,  SWITCH_DJ= 4                     // The (Julian) day of the month (before)
,  SWITCH_DG= 15                    // The (Gregorian) day of the month (after)
,  SWITCH_YY= 1582                  // The year
,  SWITCH_JD= 2'299'161             // The first Gregorian Julian Day
}; // enum SWITCH_

//----------------------------------------------------------------------------
// Calendar::Attributes
//----------------------------------------------------------------------------
protected:
int64_t                year;        // The year
uint16_t               month;       // The month, range 1..12
uint16_t               day;         // The day of the month, range 1..31
uint16_t               hour;        // The hour of the day, range 0..23
uint16_t               minute;      // The minute of the hour, range 0..59
double                 second;      // The second of the minute, range 0..(<60)

//----------------------------------------------------------------------------
// Calendar::Day of cycle to {year, month, day} lookup table
//----------------------------------------------------------------------------
public:
static const struct YMD {           // Year, month day lookup table entry
uint16_t               year;        // Year, range  0 .. 399
uint8_t                month;       // Month, range 1 .. 12
uint8_t                day;         // Day,   range 1 .. 31
}                      day2ymd[146'097]; // Day to {year, month, day} lookup

//----------------------------------------------------------------------------
// Calendar::Constructors/destructor
//----------------------------------------------------------------------------
   Calendar( void )                 // Default Constructor
{  set(); }                         // Set to current time

   Calendar(                        // Construct the Calendar using
     int64_t           year,        // The year
     unsigned int      month,       // The month, range 1..12
     unsigned int      day)         // The day of the month, range 1..31
{  setYMD(year, month, day); }

   Calendar(                        // Construct the Calendar using
     int64_t           year,        // The year
     unsigned int      month,       // The month, range 1..12
     unsigned int      day,         // The day of the month, range 1..31
     unsigned int      hour,        // The hour of the day, range 0..23
     unsigned int      minute,      // The minute of the hour, range 0..59
     double            second= 0)   // The second of the minute, range 0..(<60)
{  setYMDHMS(year, month, day, hour, minute, second); }

   Calendar(                        // Copy constructor
     const Calendar&   calendar)    // Source Calendar
{  set(calendar); }                 // Copy source Calendar

explicit
   Calendar(                        // Copy constructor
     const Julian&     julian)      // Source Julian
{  set(julian); }                   // Convert Julian to Calendar

explicit
   Calendar(                        // Copy constructor
     const Clock&      clock)       // Source Clock
{  set(clock); }                    // Convert Clock to Calendar

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   ~Calendar( void ) = default;     // Destructor

//----------------------------------------------------------------------------
// Calendar::Operators
//----------------------------------------------------------------------------
// Cast operators- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   explicit operator Clock( void ) const; // Cast Calendar to Clock

   explicit operator Julian( void ) const; // Cast Calendar to Julian

   explicit operator std::string( void ) const; // Cast Calendar to string

// Assignment operators- - - - - - - - - - - - - - - - - - - - - - - - - - - -
Calendar&                           // Resultant
   operator=(                       // Assignment operator
     const Calendar&   calendar);   // Source Calendar

Calendar&                           // Resultant
   operator=(                       // Assignment operator
     const Julian&     julian);     // Source Julian

Calendar&                           // Resultant
   operator=(                       // Assignment operator
     const Clock&      clock);      // Source Clock

// Arithmentic operators - - - - - - - - - - - - - - - - - - - - - - - - - - -
// (None provided)

// Comparison operators- - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // Resultant
   operator==(                      // Compare to
     const Calendar&   comprahend) const // Comprahend
{  return compare(comprahend) == 0; }

bool                                // Resultant
   operator!=(                      // Compare to
     const Calendar&   comprahend) const // Comprahend
{  return compare(comprahend) != 0; }

bool                                // Resultant
   operator<(                       // Compare to
     const Calendar&   comprahend) const // Comprahend
{  return compare(comprahend)  < 0; }

bool                                // Resultant
   operator<=(                      // Compare to
     const Calendar&   comprahend) const // Comprahend
{  return compare(comprahend) <= 0; }

bool                                // Resultant
   operator>(                       // Compare to
     const Calendar&   comprahend) const // Comprahend
{  return compare(comprahend)  > 0; }

bool                                // Resultant
   operator>=(                      // Compare to
     const Calendar&   comprahend) const // Comprahend
{  return compare(comprahend) >= 0; }

//----------------------------------------------------------------------------
// Calendar::Accessor methods
//----------------------------------------------------------------------------
int64_t                             // The Calendar year
   get_year( void ) const           // Get Calendar year
{  return year; }

unsigned int                        // The Calendar month
   get_month( void ) const          // Get Calendar month
{  return month; }

unsigned int                        // The Calendar day
   get_day( void ) const            // Get Calendar day
{  return day; }

unsigned int                        // The Calendar hour
   get_hour( void ) const           // Get Calendar hour
{  return hour; }

unsigned int                        // The Calendar minute
   get_minute( void ) const         // Get Calendar minute
{  return minute; }

double                              // The Calendar second
   get_second( void ) const         // Get Calendar second
{  return second; }

//----------------------------------------------------------------------------
// Calendar::Methods
//----------------------------------------------------------------------------
int                                 // Resultant (<0, 0, >0)
   compare(                         // Compare this to another Calendar
     const Calendar&   comprahend) const; // Comprahend

void
   set( void );                     // Set the Calendar from current time

void
   setYMD(                          // Set the Calendar
     int64_t           year,        // The year
     unsigned int      month,       // The month, range 1..12
     unsigned int      day)         // The day of the month, range 1..31
{  setYMDHMS(year, month, day); }

void
   setYMDHMS(                       // Set the Calendar
     int64_t           year,        // The year
     unsigned int      month,       // The month, range 1..12
     unsigned int      day,         // The day of the month, range 1..31
     unsigned int      hour= 0,     // The hour of the day, range 0..23
     unsigned int      minute= 0,   // The minute of the hour, range 0..59
     double            second= 0);  // The second of the minute, range 0..(<60)

void
   set(                             // Set the Calendar
     const Calendar&   calendar);   // From this Calendar

void
   set(                             // Set the Calendar
     const Julian&     julian);     // From this Julian

void
   set(                             // Set the Calendar
     const Clock&      clock);      // From this Clock
}; // class Calendar
_LIBPUB_END_NAMESPACE
#endif  // _LIBPUB_CALENDAR_H_INCLUDED
