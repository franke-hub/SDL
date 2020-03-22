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
//       Clock.h
//
// Purpose-
//       A Clock contains a positive time offset from some Epoch.
//
// Last change date-
//       2014/01/01
//
// Notes-
//       An Epoch is an arbitrary time origin which cannot change without
//       a machine reboot. The current Epoch began Jan 1, 1970 and provides
//       for at least microsecond clock resolution until the year 2100.
//
//----------------------------------------------------------------------------
#ifndef CLOCK_H_INCLUDED
#define CLOCK_H_INCLUDED

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Julian;

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
inline
   ~Clock( void );                  // Destructor
inline
   Clock( void );                   // Default Constructor (Current time)

inline
   Clock(                           // Copy Constructor
     const Clock&      source);     // Source

   Clock(                           // Copy Constructor
     const Julian&     source);     // Source

inline
   Clock(                           // Constructor
     double            source);     // Source (seconds since Epoch)

inline Clock&                       // Resultant
   operator=(                       // Assignment operator
     const Clock&      source);     // Source

Clock&                              // Resultant
   operator=(                       // Assignment operator
     const Julian&     source);     // Source

inline Clock&                       // Resultant
   operator=(                       // Assignment operator
     double            source);     // Source (seconds since Epoch)

//----------------------------------------------------------------------------
// Clock::Operators
//----------------------------------------------------------------------------
public:
inline Clock                        // Resultant
   operator+(                       // Add to
     const Clock&      addend) const; // Addend

inline Clock                        // Resultant
   operator-(                       // Subtract from
     const Clock&      subtrahend) const; // Subtrahend

inline Clock                        // Resultant
   operator+=(                      // Add to this
     const Clock&      addend);       // Addend

inline Clock                        // Resultant
   operator-=(                      // Subtract from this
     const Clock&      subtrahend);   // Subtrahend

inline int                          // Resultant
   operator!=(                      // Compare to
     const Clock&      comprahend) const; // Comprahend

inline int                          // Resultant
   operator<=(                      // Compare to
     const Clock&      comprahend) const; // Comprahend

inline int                          // Resultant
   operator==(                      // Compare to
     const Clock&      comprahend) const; // Comprahend

inline int                          // Resultant
   operator>=(                      // Compare to
     const Clock&      comprahend) const; // Comprahend

inline int                          // Resultant
   operator<(                       // Compare to
     const Clock&      comprahend) const; // Comprahend

inline int                          // Resultant
   operator>(                       // Compare to
     const Clock&      comprahend) const; // Comprahend

inline
   operator double( void ) const;   // Cast to double

//----------------------------------------------------------------------------
// Clock::Accessor methods
//----------------------------------------------------------------------------
public:
inline double                       // The seconds since epoch
   getTime( void ) const;           // Get seconds since epoch

//----------------------------------------------------------------------------
// Clock::Methods
//----------------------------------------------------------------------------
public:
static double                       // The number of seconds since the Epoch
   current( void );                 // Get number of seconds since the Epoch

Julian                              // The Julian
   toJulian( void ) const;          // Convert to Julian
};

#include "Clock.i"

#endif  // CLOCK_H_INCLUDED
