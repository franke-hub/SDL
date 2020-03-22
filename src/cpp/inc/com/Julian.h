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
//       Julian.h
//
// Purpose-
//       The Julian object contains the Juian date and time.
//
// Last change date-
//       2014/01/01
//
// Acronyms-
//       BCE: Before Christian (or Common) Era.
//       UTC: Coordinated Universal Time,
//            formerly known as Greenwich Mean Time (GMT).
//
// Implementation notes-
//       The Julian object contains the time in seconds before or after the
//       Julian chronologic Epoch origin: midnight January 1, 4713 BCE, UTC.
//       Note that the common notation for this date is 1/1/-4712, since
//       the common era begins 1/1/0001 rather than 1/1/0000.
//
// Notes-
//       January 1, 2000 (common era) is Day(2451545).
//
//       Julian day 1 is a Tuesday. When using modulus to get the day of the
//       week, Monday is day 0.
//
//       JuilanDate is chronological Julian calendar time, in which time
//       is counted from midnight to midnight.  Contrast this with Julian
//       astronomical time, which runs from noon to noon.  Add 12 hours
//       (43,200 seconds) to a Julian to obtain astronomical time.
//
//----------------------------------------------------------------------------
#ifndef JULIAN_H_INCLUDED
#define JULIAN_H_INCLUDED

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Clock;

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
// Julian::Attributes
//----------------------------------------------------------------------------
protected:
double                 time;        // Seconds since epoch

//----------------------------------------------------------------------------
// Julian::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum                                // Constants for parameterization
{  SECONDS_PER_DAY= 86400           // Number of seconds per day
};

//----------------------------------------------------------------------------
// Julian::Constants
//----------------------------------------------------------------------------
public:
static const Julian    UTC0001;     // Jan 1, 0001 (Julian calendar)
static const Julian    UTC1601;     // Jan 1, 1601 (Gregorian calendar)
static const Julian    UTC1900;     // Jan 1, 1900 (Gregorian calendar)
static const Julian    UTC1970;     // Jan 1, 1970 (PC Epoch)
static const Julian    UTC2000;     // Jan 1, 2000

//----------------------------------------------------------------------------
// Julian::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~Julian( void );                 // Destructor
inline
   Julian( void );                  // Default Constructor (current time)

inline
   Julian(                          // Copy Constructor
     const Julian&     source);     // Source Julian

   Julian(                          // Copy Constructor
     const Clock&      source);     // Source Clock

inline
   Julian(                          // Constructor
     double            source);     // Source Julian second

//----------------------------------------------------------------------------
// Julian::Operators
//----------------------------------------------------------------------------
public:
inline
   operator double( void ) const;   // Cast to double (seconds since epoch)

inline Julian&                      // Resultant
   operator=(                       // Assignment operator
     const Julian&     source);     // Source Julian

Julian&                             // Resultant
   operator=(                       // Assignment operator
     const Clock&      source);     // Source Clock

inline Julian&                      // Resultant
   operator=(                       // Assignment operator
     double            source);     // Source Julian second

inline Julian                       // Resultant
   operator+(                       // Add to
     const Julian&     addend) const; // Addend

inline Julian                       // Resultant
   operator-(                       // Subtract from
     const Julian&     subtrahend) const; // Subtrahend

inline Julian&                      // Resultant
   operator+=(                      // Add to this
     const Julian&     addend);     // Addend

inline Julian&                      // Resultant
   operator-=(                      // Subtract from this
     const Julian&     subtrahend); // Subtrahend

inline int                          // Resultant
   operator!=(                      // Compare to
     const Julian&     comprahend) const; // Comprahend

inline int                          // Resultant
   operator<=(                      // Compare to
     const Julian&     comprahend) const; // Comprahend

inline int                          // Resultant
   operator==(                      // Compare to
     const Julian&     comprahend) const; // Comprahend

inline int                          // Resultant
   operator>=(                      // Compare to
     const Julian&     comprahend) const; // Comprahend

inline int                          // Resultant
   operator<(                       // Compare to
     const Julian&     comprahend) const; // Comprahend

inline int                          // Resultant
   operator>(                       // Compare to
     const Julian&     comprahend) const; // Comprahend

//----------------------------------------------------------------------------
// Julian::Accessor methods
//----------------------------------------------------------------------------
public:
inline double                       // The Julian day (Days since epoch)
   getDate( void ) const;           // Get Julian day

inline double                       // The Julian time (Seconds since epoch)
   getTime( void ) const;           // Get Julian time

// These methods are present only to avoid DLL library problems
static double                       // The UTC1601 Julian time
   getUTC1601Time( void );          // Get UTC1601 Julian time

static double                       // The UTC1970 Julian time
   getUTC1970Time( void );          // Get UTC1970 Julian time

//----------------------------------------------------------------------------
// Julian::Methods
//----------------------------------------------------------------------------
public:
static double                       // The Julian second (since epoch)
   current( void );                 // Get Julian second

Clock                               // The Clock
   toClock( void ) const;           // Convert to Clock
}; // class Julian

#include "Julian.i"

#endif  // JULIAN_H_INCLUDED
