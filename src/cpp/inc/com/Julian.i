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
//       Julian.i
//
// Purpose-
//       Julian inline methods.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       Julian::~Julian( void )
//
// Function-
//       Destructor
//
//----------------------------------------------------------------------------
   Julian::~Julian( void )          // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::Julian( void )
//
// Function-
//       Default constructor
//
//----------------------------------------------------------------------------
   Julian::Julian( void )           // Constructor
{
   time= current();
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::Julian(const Julian&)
//
// Function-
//       Copy constructor.
//
//----------------------------------------------------------------------------
   Julian::Julian(                  // Copy constructor
     const Julian&     source)      // Source
{
   time= source.time;
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::Julian(double)
//
// Function-
//       Constructor
//
//----------------------------------------------------------------------------
   Julian::Julian(                  // Constructor
     double            source)      // The Julian second
{
   time= source;
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::operator double
//
// Function-
//       Return the Julian second
//
//----------------------------------------------------------------------------
   Julian::operator double( void ) const // Get Julian second
{
   return time;
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::operator=
//
// Function-
//       Assign to this Julian.
//
//----------------------------------------------------------------------------
Julian&                             // Resultant
   Julian::operator=(               // Assign to this
     const Julian&     source)      // Source Julian
{
   time= source.time;
   return *this;
}

Julian&                             // Resultant
   Julian::operator=(               // Assign to this
     double            source)      // Source Julian second
{
   time= source;
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::operator+
//
// Function-
//       Add to this Julian.
//
//----------------------------------------------------------------------------
Julian                              // Resultant
   Julian::operator+(               // Add to this
     const Julian&     addend) const // Addend
{
   Julian resultant(*this);

   resultant.time += addend.time;
   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::operator-
//
// Function-
//       Subtract from this Julian.
//
//----------------------------------------------------------------------------
Julian                              // Resultant
   Julian::operator-(               // Subtract from this
     const Julian&     subtrahend) const // Subtrahend
{
   Julian resultant(*this);

   resultant.time -= subtrahend.time;
   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::operator+=
//
// Function-
//       Add to this Julian.
//
//----------------------------------------------------------------------------
Julian&                             // Resultant
   Julian::operator+=(              // Add to this
     const Julian&     addend)      // Addend
{
   time += addend.time;
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::operator-=
//
// Function-
//       Subtract from this Julian.
//
//----------------------------------------------------------------------------
Julian&                             // Resultant
   Julian::operator-=(              // Subtract from this
     const Julian&     subtrahend)  // Subtrahend
{
   time -= subtrahend.time;
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::operator!=
//
// Function-
//       Compare Julian objects.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Julian::operator!=(              // Compare to
     const Julian&     comprahend) const // Comprahend
{
   return (time != comprahend.time);
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::operator<
//
// Function-
//       Compare Julian objects.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Julian::operator<(               // Compare to
     const Julian&     comprahend) const // Comprahend
{
   return (time < comprahend.time);
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::operator<=
//
// Function-
//       Compare Julian objects.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Julian::operator<=(              // Compare to
     const Julian&     comprahend) const // Comprahend
{
   return (time <= comprahend.time);
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::operator==
//
// Function-
//       Compare Julian objects.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Julian::operator==(              // Compare to
     const Julian&     comprahend) const // Comprahend
{
   return (time == comprahend.time);
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::operator>
//
// Function-
//       Compare Julian objects.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Julian::operator>(               // Compare to
     const Julian&     comprahend) const // Comprahend
{
   return (time > comprahend.time);
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::operator>=
//
// Function-
//       Compare Julian objects.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Julian::operator>=(              // Compare to
     const Julian&     comprahend) const // Comprahend
{
   return (time >= comprahend.time);
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::getDate
//
// Function-
//       Return the Julian day number.
//
//----------------------------------------------------------------------------
double                              // The integer Julian day number
   Julian::getDate( void ) const    // Get integer Julian day number
{
   return time / double(SECONDS_PER_DAY);
}

//----------------------------------------------------------------------------
//
// Method-
//       Julian::getTime
//
// Function-
//       Return the Julian second.
//
//----------------------------------------------------------------------------
double                              // The Julian second
   Julian::getTime( void ) const    // Get Julian second
{
   return time;
}

