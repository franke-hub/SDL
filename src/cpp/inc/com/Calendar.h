//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Calendar.h
//
// Purpose-
//       The Calendar object defines a month, day and year calendar.
//
// Last change date-
//       2007/01/01
//
// Notes-
//       The default calendar is the English and American Gregorian calendar.
//       The associated time is always UTC, Coordinated Universal Time.
//       There is NO provision for local time or daylight savings time.
//
//       The Gregorian calendar, as specified by Pope Gregory XIII, specifies
//       that the Julian calendar is used on or before October 4, 1582 and
//       the Gregorian calendar thereafter.
//       Thursday, October 4, 1582 is followed by Friday, October 15, 1582.
//       Julian day 299160 corresponds to Thursday, October 4, 1582.
//
//       The Gregorian calendar was adopted on different dates in different
//       countries.  For England and the colonies, the Gregorian calendar was
//       adopted on September 2, 1752.  This is Julian day 2361221.
//       Wednesday, September 2, 1752 was followed by Thursday, September 14th.
//
//       This calendar uses the earliest switchover date, October 4, 1582.
//       All years before the switchover date divisible by four are leap years
//       in which February contains 29 days.  Note that year 0 is 1 BC.
//
//----------------------------------------------------------------------------
#ifndef CALENDAR_H_INCLUDED
#define CALENDAR_H_INCLUDED

#include <stdint.h>

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Clock;
class Julian;

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
// Calendar::Attributes
//----------------------------------------------------------------------------
private:
int64_t                year;        // The year
uint16_t               month;       // The month, range 1..12
uint16_t               day;         // The day of the month, range 1..31
uint16_t               hour;        // The hour of the day, range 0..23
uint16_t               minute;      // The minute of the hour, range 0..59
uint16_t               second;      // The second of the minute, range 0..59
uint16_t               millisecond; // The millisecond, range 0..999

//----------------------------------------------------------------------------
// Calendar::Constructors
//----------------------------------------------------------------------------
public:
   ~Calendar( void );               // Calendar
   Calendar( void );                // Default Constructor

   Calendar(                        // Copy Constructor
     const Calendar&   source);     // Source

   Calendar(                        // Constructor
     const Clock&      source);     // Source

   Calendar(                        // Constructor
     const Julian&     source);     // Source

inline Calendar&                   // Resultant
   operator=(                       // Assignment operator
     const Calendar&   source);     // Source

Calendar&                           // Resultant
   operator=(                       // Assignment operator
     const Clock&      source);     // Source

Calendar&                           // Resultant
   operator=(                       // Assignment operator
     const Julian&     source);     // Source

//----------------------------------------------------------------------------
// Calendar::Operators
//----------------------------------------------------------------------------
public:
inline int                          // Resultant
   operator!=(                      // Compare to
     const Calendar&   comprahend) const; // Comprahend

inline int                          // Resultant
   operator<(                       // Compare to
     const Calendar&   comprahend) const; // Comprahend

inline int                          // Resultant
   operator<=(                      // Compare to
     const Calendar&   comprahend) const; // Comprahend

inline int                          // Resultant
   operator==(                      // Compare to
     const Calendar&   comprahend) const; // Comprahend

inline int                          // Resultant
   operator>(                       // Compare to
     const Calendar&   comprahend) const; // Comprahend

inline int                          // Resultant
   operator>=(                      // Compare to
     const Calendar&   comprahend) const; // Comprahend

//----------------------------------------------------------------------------
// Calendar::Methods
//----------------------------------------------------------------------------
public:
int                                 // Resultant (<0, 0, >0)
   compare(                         // Compare this to another Calendar
     const Calendar&   comprahend) const; // Comprahend

inline int64_t                      // The year
   getYear( void) const;            // Get year

inline unsigned int                 // The month of year    (range 1..12)
   getMonth( void) const;           // Get month of year    (range 1..12)

inline unsigned int                 // The day of month     (range 1..31)
   getDay( void) const;             // Get day of month     (range 1..31)

inline unsigned int                 // The hour of day      (range 0..23)
   getHour( void) const;            // Get hour of day      (range 0..23)

inline unsigned int                 // The minute of hour   (range 0..59)
   getMinute( void) const;          // Get minute of hour   (range 0..59)

inline unsigned int                 // The second of minute (range 0..59)
   getSecond( void) const;          // Get second of minute (range 0..59)

inline unsigned int                 // The millisecond of second
   getMillisecond( void) const;     // Get millisecond of second

void
   set( void );                     // Set the Calendar to the current date

void
   set(                             // Set the Calendar
     const Calendar&   source);     // From this Calendar

void
   set(                             // Set the Calendar
     const Julian&     time);       // From this Julian

void
   setYMDHMSN(                      // Set the Calendar
     int64_t           year,        // The year
     unsigned int      month,       // The month, range 1..12
     unsigned int      day,         // The day of the month, range 1..31
     unsigned int      hour,        // The hour of the day, range 0..23
     unsigned int      minute,      // The minute of the hour, range 0..59
     unsigned int      second,      // The second of the minute, range 0..59
     unsigned int      millisecond = 0); // The millisecond, range 0..999

void
   setYMD(                          // Set the Calendar (Time 00:00)
     int64_t           year,        // The year
     unsigned int      month,       // The month, range 1..12
     unsigned int      day);        // The day of the month, range 1..31

Clock                               // The resultant Clock
   toClock( void ) const;           // Get a Clock from this Calendar

Julian                              // The resultant Julian
   toJulian( void ) const;          // Get a Julian from this Calendar
}; // class Calendar

#include "Calendar.i"

#endif  // CALENDAR_H_INCLUDED
