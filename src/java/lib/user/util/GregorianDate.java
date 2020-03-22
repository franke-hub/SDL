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
//       user.util.GregorianDate.java
//
// Purpose-
//       Define the GregorianDate class.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package user.util;

//----------------------------------------------------------------------------
//
// Class-
//       GregorianDate
//
// Purpose-
//       Gregorian calendar date.
//
// Related classes-
//       ReferenceTime    // The Reference date and time of day.
//
// Notes-
//       The Gregorian calendar was adopted on different dates in different
//       countries.  Here we use the adoption dates of England and the
//       Colonies: September 2, 1752.  During that year September 14th
//       followed September 2nd.  September 2nd was a Wednesday and
//       September 14th a Thursday.
//
//       This calendar reports dates on or before September 2, 1752 as Julian
//       dates.  All years before 1752 divisible by four are leap years
//       in which February contains 29 days.  The year 0 is 1 BC.
//
//----------------------------------------------------------------------------
public class GregorianDate
{
//----------------------------------------------------------------------------
//
// Method-
//       GregorianDate.GregorianDate
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   GregorianDate( )                 // Constructor
{
   year=  0;
   month= 1;
   day=   1;
}

//----------------------------------------------------------------------------
//
// Method-
//       GregorianDate.getYear
//
// Purpose-
//       Extract the year.
//
//----------------------------------------------------------------------------
public long                         // The year
   getYear( )                       // Extract the year
{
   return year;
}

//----------------------------------------------------------------------------
//
// Method-
//       GregorianDate.getMonthOfYear
//
// Purpose-
//       Extract the month.
//
//----------------------------------------------------------------------------
public int                          // The month of the year, range 1..12
   getMonthOfYear( )                // Extract the month of the year
{
   return month;
}

//----------------------------------------------------------------------------
//
// Method-
//       GregorianDate.getDayOfWeek
//
// Purpose-
//       Extract the day of the week, Monday is 0, Tuesday is 1 .. Sunday is 6
//
//----------------------------------------------------------------------------
public int                          // The day of the week, range 0..6
   getDayOfWeek( )                  // Extract the day of the week
   throws Exception
{
   long                day;         // The Reference Day
   ReferenceDate       j;           // Get the Reference date

   j= getReferenceDate();           // Get the Reference date
   day= j.getDay() % 7;             // Get the day of the week
   if( day < 0 )                    // If negative Reference Date, not Monday
     day= 7 + day;                  // Adjust (day is negative)

   return (int)day;
}

//----------------------------------------------------------------------------
//
// Method-
//       GregorianDate.getDayOfMonth
//
// Purpose-
//       Extract the day of the month.
//
//----------------------------------------------------------------------------
public int                          // The day of the month, range 1..31
   getDayOfMonth( )                 // Extract the day of the month
{
   return day;
}

//----------------------------------------------------------------------------
//
// Method-
//       GregorianDate.isLeapYear
//
// Purpose-
//       Is this a Leap year?
//
//----------------------------------------------------------------------------
public static boolean               // TRUE if leap year
   isLeapYear(                      // Is this a leap year?
     long            year)          // The year
   throws Exception
{
   if( year < 1752 )
   {
     if( (year % 4 ) == 0 )
       return true;

     return false;
   }

   if( (year % 4 ) == 0 && ((year % 100) != 0 || (year % 400) == 0 ) )
     return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       GregorianDate.setMonthDayYear
//
// Purpose-
//       Set the date given month, day, year
//
//----------------------------------------------------------------------------
public void
   setMonthDayYear(                 // Set the date
     int               month,       // The month of the year
     int               day,         // The day of the month
     long              year)        // The year
   throws Exception
{
   setYearMonthDay(year, month, day);
}

//----------------------------------------------------------------------------
//
// Method-
//       GregorianDate.setDayMonthYear
//
// Purpose-
//       Set the date given day, month, year
//
//----------------------------------------------------------------------------
public void
   setDayMonthYear(                 // Set the date
     int               day,         // The day of the month
     int               month,       // The month of the year
     long              year)        // The year
   throws Exception
{
   setYearMonthDay(year, month, day);
}

//----------------------------------------------------------------------------
//
// Method-
//       GregorianDate.setYearMonthDay
//
// Purpose-
//       Set the date given year, month, day
//
//----------------------------------------------------------------------------
public void
   setYearMonthDay(                 // Set the date
     long              year,        // The year
     int               month,       // The month of the year
     int               day)         // The day of the month
   throws Exception
{
   int                 leap;        // 0 if ordinary, 1 if leap year

   leap= 0;
   if( isLeapYear(year) )
     leap= 1;

   if( month < 1 || month > 12 )
     throw new Exception("Invalid Gregorian month");
   if( day < 1 || day > daysPerMonth[leap][month-1] )
     throw new Exception("Invalid Gregorian day of month");
   if( year == 1752 && month == 9 && day > 2 && day < 14 )
     throw new Exception("September 3 through 13, 1752 " +
                         "do not exist in the Gregorian calendar");

   this.month= month;
   this.day=   day;
   this.year=  year;
}

//----------------------------------------------------------------------------
//
// Method-
//       GregorianDate.set
//
// Purpose-
//       Set the date from a ReferenceDate.
//
//----------------------------------------------------------------------------
public void
   set(                             // Set the date
     ReferenceDate     julian)      // Given the Reference date
{
   long                julianDay;   // The Julian day
   long                century;     // The Julian century
   long                cycles;      // Number of complete four-year cycles
   int                 wday;        // Working day
   long                wyear;       // Working year

   int                 isLeap;      // 0 if ordinary, 1 if leap year

   int                 x;

   julianDay= julian.getDay();      // Get the Julian day

   if( julianDay > 2361221 )        // If on or after 09/02/1752
   {
     century= (long)((double)(julianDay-1684595) / 36524.25);
     julianDay += ((century*3)/4) - 2;
   }

   cycles= julianDay/1461;          // The number of complete 4-year cycles
   wyear= cycles*4;                 // The number of years
   wday= (int)(julianDay%1461);     // The number of days left in cycle

   if( julianDay < 0 )              // If before 01/01/(-4712)
   {
     wyear= wyear-4;
     wday= 1461 + wday;             // (wday is negative)
   }

   for(x= 0; x<4 && 365*(x+1) < wday; x++)
   {
   }

   if( x == 0 && wyear <= 6464 )
   {
     isLeap= 1;
     wday++;
   }

   else if( x == 0 && wyear > 6464
            && (((wyear-4712) % 100) != 0 || ((wyear-4712) % 400) == 0) )
   {
     isLeap= 1;
     wday++;
   }

   else if( x == 0 && wyear > 6464
            && (((wyear-4712) % 100) == 0 || ((wyear-4712) % 400) != 0) )
   {
     isLeap= 0;
     if( wday < dayOfYear[0][2] )
       wday++;
   }

   else
   {
     isLeap= 0;
     wday  -= (365*x);
     wyear += x;
   }

   for(x= 2; x<13 && wday > dayOfYear[isLeap][x-1]; x++)
   {
   }

   month= x-1;
   day=   wday - dayOfYear[isLeap][month-1];
   year=  wyear - 4712;
}

//----------------------------------------------------------------------------
//
// Method-
//       GregorianDate.getReferenceDate
//
// Purpose-
//       Set the date given the specified Gregorian calendar date.
//
//----------------------------------------------------------------------------
public ReferenceDate                // The ReferenceDate
   getReferenceDate( )              // Extract the associated Reference date
   throws Exception
{
   long                resultant;   // The resultant Reference Day

   long                century;     // Working century
   long                wyear;       // Working year
   int                 leap;        // 0 if ordinary, 1 if leap year

   wyear= year + 4712;              // Day 0 is Jan 1, 4713 BCE
   if( wyear < -13343998895911L || wyear > 13343998895911L )
     throw new Exception("Gregorian year out of range for Reference day");

   // Compute the Reference date
   if( wyear < 0 )
   {
     resultant= 365 * wyear + wyear/4 - 1;
     leap= 0;
     if( (wyear%4) == 0 )
       leap= 1;
   }
   else
   {
     resultant= 365 * wyear + wyear/4;
     leap= 0;
     if( (wyear%4) == 0 )
     {
       leap= 1;
       resultant--;
     }
   }

   resultant += dayOfYear[leap][month-1] + day;
   if( resultant <= 2361221 )       // If Gregorian date before 09/02/1752
     return new ReferenceDate(resultant); // No adjustment required

   wyear= year - 300;
   if( month < 3 )
     wyear--;

   century= wyear/100;
   resultant= resultant - ((century*3)/4) - 1;
   return new ReferenceDate(resultant);
}

//----------------------------------------------------------------------------
// GregorianDate.Static attributes
//----------------------------------------------------------------------------
static public final int
                     dayOfYear[][]= {
    {   0,  31,  59,  90, 120, 151, 181, 212, 243, 273, 304, 334}, // Ordinary
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335}  // Leap
    };

static public final int
                     daysPerMonth[][]= {
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, // Ordinary year
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}  // Leap year
    };

//----------------------------------------------------------------------------
// GregorianDate.Attributes
//----------------------------------------------------------------------------
int                  month;         // Month (range 1..12)
int                  day;           // Day   (range 1..31)
long                 year;          // Year  (unrestricted)
} // Class GregorianDate

