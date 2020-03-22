//----------------------------------------------------------------------------
//
//       Copyright (C) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       user.util.ReferenceDate.java
//
// Purpose-
//       Define the ReferenceDate class.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package user.util;

//----------------------------------------------------------------------------
//
// Class-
//       ReferenceDate
//
// Purpose-
//       A standardized date class, containing a long count of 24 hour days.
//
// Related classes-
//       ReferenceTime              // The Reference date and time of day.
//
// Notes-
//       Day(0) for a ReferenceDate Object is January 1, 4713 BCE.
//       January 1, 2000 (common era) is Day(2451545).
//
//----------------------------------------------------------------------------
public class ReferenceDate
{
//----------------------------------------------------------------------------
//
// Method-
//       ReferenceDate.ReferenceDate
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
public
   ReferenceDate( )                 // Default constructor
{
   day= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       ReferenceDate.ReferenceDate
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   ReferenceDate(                   // Constructor
     long              day)         // The Reference day
{
   this.day= day;
}

//----------------------------------------------------------------------------
//
// Method-
//       ReferenceDate.getDay
//
// Purpose-
//       Extract the Reference day.
//
//----------------------------------------------------------------------------
public long                         // The Reference day
   getDay( )                        // Extract the Reference day
{
   return day;
}

//----------------------------------------------------------------------------
//
// Method-
//       ReferenceDate.setDay
//
// Purpose-
//       Set the Reference day.
//
//----------------------------------------------------------------------------
public void
   setDay(                          // Set the Reference day
     long              day)         // The day to set
{
   this.day= day;
}

//----------------------------------------------------------------------------
//
// Method-
//       ReferenceDate.set
//
// Purpose-
//       Set the Reference day.
//
//----------------------------------------------------------------------------
public long                         // The resultant Reference day
   set(                             // Set the Reference day
     ReferenceDate     date)        // From another ReferenceDate
{
   day= date.day;
   return day;
}

//----------------------------------------------------------------------------
// ReferenceDate.Attributes
//----------------------------------------------------------------------------
long                   day;         // The Reference day
} // Class ReferenceDate

