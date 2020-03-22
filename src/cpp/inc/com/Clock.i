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
//       Clock.i
//
// Purpose-
//       Clock inline methods.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       Clock::~Clock
//
// Function-
//       Destructor
//
//----------------------------------------------------------------------------
   Clock::~Clock( void )            // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::Clock( void )
//
// Function-
//       Constructor
//
//----------------------------------------------------------------------------
   Clock::Clock( void )             // Constructor
{
   time= current();
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::Clock(const Clock&)
//
// Function-
//       Copy constructor.
//
//----------------------------------------------------------------------------
   Clock::Clock(                    // Copy constructor
     const Clock&      source)      // Source
{
   time= source.time;
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::Clock(double)
//
// Function-
//       Constructor
//
//----------------------------------------------------------------------------
   Clock::Clock(                    // Constructor
     double            source)      // Source
{
   time= source;
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::operator=
//
// Function-
//       Assignment operator
//
//----------------------------------------------------------------------------
Clock&                              // Resultant
   Clock::operator=(                // Assignment operator
     const Clock&      source)      // Source
{
   time= source.time;
   return *this;
}

Clock&                              // Resultant
   Clock::operator=(                // Assignment operator
     double            source)      // Source
{
   time= source;
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::operator double
//
// Function-
//       Cast to double.
//
//----------------------------------------------------------------------------
   Clock::operator double( void ) const // Cast to double
{
   return time;
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::getTime
//
// Function-
//       Extract the time (seconds since Epoch) value.
//
//----------------------------------------------------------------------------
double                              // The number of seconds since the Epoch
   Clock::getTime( void ) const     // Get number of seconds since the Epoch
{
   return time;
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::operator+
//
// Function-
//       Add to this Clock.
//
//----------------------------------------------------------------------------
Clock                               // Resultant
   Clock::operator+(                // Add to this
     const Clock&      addend) const // Addend
{
   Clock result(*this);
   result.time += addend.time;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::operator-
//
// Function-
//       Subtract from this Clock.
//
//----------------------------------------------------------------------------
Clock                               // Resultant
   Clock::operator-(                // Subtract from this
     const Clock&      subtrahend) const // Subtrahend
{
   Clock result(*this);
   result.time -= subtrahend.time;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::operator+=
//
// Function-
//       Add to this Clock.
//
//----------------------------------------------------------------------------
Clock                               // Resultant
   Clock::operator+=(               // Add to this
     const Clock&      addend)      // Addend
{
   time += addend.time;
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::operator-=
//
// Function-
//       Subtract from this Clock.
//
//----------------------------------------------------------------------------
Clock                               // Resultant
   Clock::operator-=(               // Subtract from this
     const Clock&      subtrahend)  // Subtrahend
{
   time -= subtrahend.time;
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::operator!=
//
// Function-
//       Compare two Clocks.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Clock::operator!=(               // Compare to
     const Clock&      comprahend) const // Comprahend
{
   return (time != comprahend.time);
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::operator<=
//
// Function-
//       Compare two Clocks.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Clock::operator<=(               // Compare to
     const Clock&      comprahend) const // Comprahend
{
   return (time <= comprahend.time);
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::operator==
//
// Function-
//       Compare two Clocks.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Clock::operator==(               // Compare to
     const Clock&      comprahend) const // Comprahend
{
   return (time == comprahend.time);
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::operator>=
//
// Function-
//       Compare two Clocks.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Clock::operator>=(               // Compare to
     const Clock&      comprahend) const // Comprahend
{
   return (time >= comprahend.time);
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::operator<
//
// Function-
//       Compare two Clocks.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Clock::operator<(                // Compare to
     const Clock&      comprahend) const // Comprahend
{
   return (time <= comprahend.time);
}

//----------------------------------------------------------------------------
//
// Method-
//       Clock::operator>
//
// Function-
//       Compare two Clocks.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Clock::operator>(                // Compare to
     const Clock&      comprahend) const // Comprahend
{
   return (time > comprahend.time);
}

