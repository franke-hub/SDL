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
//       user.util.ReferenceTime.java
//
// Purpose-
//       Extend ReferenceDate with time of day (in nanoseconds).
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package user.util;

//----------------------------------------------------------------------------
//
// Class-
//       ReferenceTime
//
// Purpose-
//       Extend ReferenceDate with time of day (in nanoseconds).
//
// Related classes-
//       ReferenceDate              // The Reference date
//
// Notes-
//       The ReferenceTime begins at midnight GMT on January 1, 4713 BCE.
//       (BCE is the abbreviation for Before Christion Era.)
//
//       By convention, ReferenceDate and ReferenceTime are always stored in
//       UTC, Coordinated Univeral Time.
//       This is also known as GMT, or Greenwich Mean Time.
//
//       ReferenceTime is also known as Julian calendar time, in which time
//       is counted from midnight to midnight.  Contrast this with Julian
//       astronomical time, which runs from noon to noon.  You must add 12
//       hours to a ReferenceTime to obtain the corresponding astronomical
//       time.
//
//----------------------------------------------------------------------------
public class ReferenceTime extends ReferenceDate
{
//----------------------------------------------------------------------------
//
// Method-
//       ReferenceTime.ReferenceTime
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
public
   ReferenceTime( )                 // Default constructor
{
   super();
   tod= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       ReferenceTime.ReferenceTime(days, nanoSeconds)
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   ReferenceTime(                   // Constructor
     long              days,        // Number of days
     long              nanoseconds) // Number of nanoseconds
   throws Exception                 // If invalid time of day
{
   day= days;
   setNanosecond(nanoseconds);
}

//----------------------------------------------------------------------------
//
// Method-
//       ReferenceTime.ReferenceTime(ReferenceDate)
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   ReferenceTime(                   // Constructor
     ReferenceDate     date,        // Given date
     long              nanosecond)  // and time of day (in nanoseconds)
   throws Exception                 // If invalid time of day
{
   day= date.day;
   setNanosecond(nanosecond);
}

//----------------------------------------------------------------------------
//
// Method-
//       ReferenceTime.ReferenceTime(ReferenceDate)
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   ReferenceTime(                   // Constructor
     ReferenceDate     date)        // Given date (time 00:00)
{
   day= date.day;
   tod= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       ReferenceTime.getNanoseconds
//
// Purpose-
//       Extract the time of day, in nanoseconds
//
//----------------------------------------------------------------------------
public long                         // The time of day, in nanoseconds
   getNanosecond( )                 // Extract the Reference time of day
{
   return tod;
}

//----------------------------------------------------------------------------
//
// Method-
//       ReferenceTime.setNanosecond
//
// Purpose-
//       Set the time of day, in nanoseconds
//
//----------------------------------------------------------------------------
public void
   setNanosecond(                   // Extract the Reference time of day
     long              tod)         // Time of day, in nanoseconds
   throws Exception                 // If invalid time of day
{
   if( tod < 0 )
     throw new Exception("Cannot setNanosecond < 0");

   if( tod >= nanosecondsPerDay )
     throw new Exception("Cannot setNanosecond >= " + nanosecondsPerDay);

   this.tod= tod;
}

//----------------------------------------------------------------------------
//
// Method-
//       ReferenceTime.toDouble
//
// Purpose-
//       Convert to a double.
//
//----------------------------------------------------------------------------
public double                       // The Time
   toDouble( )                      // Extract the Time
{
   double              resultant;

   resultant= day;
   resultant += (double)tod / (double)nanosecondsPerDay; // Valid + or -
   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       ReferenceTime.set
//
// Purpose-
//       Set the date and time of day.
//
//----------------------------------------------------------------------------
public void
   set(                             // Set date and time of day
     double            time)        // Fractional date
   throws Exception                 // If cannot convert to long
{
   double              fday;        // Fractional date
   double              ftod;        // Fractional time of day

   if( time > (double)0x7fffffffffffffffL // If too large (positive)
       || time < -(double)0x7fffffffffffffffL ) // or too large (negative)
     throw new Exception("Time range error");

   fday= java.lang.Math.floor(time);// The next greater day
   ftod= time - fday;               // The used fraction of the day
   if( time < 0.0 && ftod < 0.0 )
   {
     fday--;
     ftod= 1.0 + ftod;
   }

   this.day= (long)fday;
   this.tod= (long)((double)nanosecondsPerDay * ftod);

   if( this.tod == nanosecondsPerDay )
   {
     this.day++;
     this.tod= 0;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ReferenceTime.add
//
// Purpose-
//       Return the sum of this ReferenceTime + a specified time increment.
//
//----------------------------------------------------------------------------
public ReferenceTime                // The resultant sum
   add(                             // Add this ReferenceTime
     ReferenceTime     increment)   // + This time increment
{
   ReferenceTime       resultant= new ReferenceTime();

   resultant.day= this.day + increment.day;
   resultant.tod= this.tod + increment.tod;

   while( resultant.tod > nanosecondsPerDay )
   {
     resultant.day++;
     resultant.tod -= nanosecondsPerDay;
   }

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       ReferenceTime.sub
//
// Purpose-
//       Return the difference between this and another ReferenceTime.
//
//----------------------------------------------------------------------------
public ReferenceTime                // The resultant difference
   sub(                             // Subtract from this ReferenceTime
     ReferenceTime     subtrahend)  // This ReferenceTime
{
   ReferenceTime       resultant= new ReferenceTime();

   resultant.day= this.day - subtrahend.day;
   resultant.tod= this.tod;

   while( resultant.tod < subtrahend.tod )
   {
     resultant.day--;
     resultant.tod += nanosecondsPerDay;
   }
   resultant.tod -= subtrahend.tod;

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       ReferenceTime.compare
//
// Purpose-
//       Compare this ReferenceTime and another ReferenceTime
//
//----------------------------------------------------------------------------
public int                          // Resultant{ -1, 0, +1 } for{ <, == , > }
   compare(                         // Compare with this ReferenceTime
     ReferenceTime     comprahend)  // This ReferenceTime
{
   if( this.day < comprahend.day )
     return -1;
   if( this.day > comprahend.day )
     return +1;

   if( this.tod < comprahend.tod )
     return -1;
   if( this.tod > comprahend.tod )
     return +1;

   return 0;
}

//----------------------------------------------------------------------------
// ReferenceTime.Static attributes
//----------------------------------------------------------------------------
public static final long
                       nanosecondsPerDay= 86400000000000L; // The number of
                                    // nanoseconds in a day
public static final long
                       nanosecondsPerHour= 3600000000000L; // The number of
                                    // nanoseconds in an hour
public static final long
                       nanosecondsPerMinute= 60000000000L; // The number of
                                    // nanoseconds in a minute
public static final long
                       nanosecondsPerSecond=  1000000000L; // The number of
                                    // nanoseconds in a second

//----------------------------------------------------------------------------
// ReferenceTime.Attributes
//----------------------------------------------------------------------------
long                   tod;         // Time of day, in nanoseconds
                                    // ValueRange 0..(nanosecondsPerDay-1)
} // Class ReferenceTime

