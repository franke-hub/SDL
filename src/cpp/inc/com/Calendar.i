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
//       Calendar.i
//
// Purpose-
//       Calendar inline methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       Calendar::operator!=
//
// Function-
//       Compare Calendar objects.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Calendar::operator!=(          // Compare to
     const Calendar&   comprahend) const // Comprahend
{
   return (compare(comprahend) != 0);
}

//----------------------------------------------------------------------------
//
// Method-
//       Calendar::operator<
//
// Function-
//       Compare Calendar objects.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Calendar::operator<(             // Compare to
     const Calendar&   comprahend) const // Comprahend
{
   return (compare(comprahend) < 0);
}

//----------------------------------------------------------------------------
//
// Method-
//       Calendar::operator<=
//
// Function-
//       Compare Calendar objects.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Calendar::operator<=(          // Compare to
     const Calendar&   comprahend) const // Comprahend
{
   return (compare(comprahend) <= 0);
}

//----------------------------------------------------------------------------
//
// Method-
//       Calendar::operator==
//
// Function-
//       Compare Calendar objects.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Calendar::operator==(            // Compare to
     const Calendar&   comprahend) const // Comprahend
{
   return (compare(comprahend) == 0);
}

//----------------------------------------------------------------------------
//
// Method-
//       Calendar::operator>
//
// Function-
//       Compare Calendar objects.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Calendar::operator>(             // Compare to
     const Calendar&   comprahend) const // Comprahend
{
   return (compare(comprahend) > 0);
}

//----------------------------------------------------------------------------
//
// Method-
//       Calendar::operator>=
//
// Function-
//       Compare Calendar objects.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Calendar::operator>=(            // Compare to
     const Calendar&   comprahend) const // Comprahend
{
   return (compare(comprahend) >= 0);
}

//----------------------------------------------------------------------------
//
// Method-
//       Calendar::get
//
// Function-
//       Calendar accessor functions.
//
//----------------------------------------------------------------------------
int64_t                             // The year
   Calendar::getYear( void) const   // Get year
{
   return year;
}

unsigned int                        // The month of year
   Calendar::getMonth( void) const  // Get month of year
{
   return month;
}

unsigned int                        // The day of month
   Calendar::getDay( void) const    // Get day of month
{
   return day;
}

unsigned int                        // The hour of day
   Calendar::getHour( void) const   // Get hour of day
{
   return hour;
}

unsigned int                        // The minute of hour
   Calendar::getMinute( void) const // Get minute of hour
{
   return minute;
}

unsigned int                        // The second of minute
   Calendar::getSecond( void) const // Get second of minute
{
   return second;
}

unsigned int                        // The millisecond of second
   Calendar::getMillisecond( void) const // Get millisecond of second
{
   return millisecond;
}

