//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Calendar.cpp
//
// Purpose-
//       Calendar object methods.
//
// Last change date-
//       2020/10/02
//
//----------------------------------------------------------------------------
#include <com/Clock.h>
#include <com/Debug.h>
#include <com/Julian.h>

#include "com/Calendar.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

enum                                // Generic enum
{  MILLISECONDS_PER_SECOND= 1000    // Milliseconds per second
,  SECONDS_PER_DAY=    86400        // Seconds per day
,  SECONDS_PER_HOUR=    3600        // Seconds per hour
,  SECONDS_PER_MINUTE=    60        // Seconds per minute
,  MINUTES_PER_HOUR=      60        // Minutes per second
,  HOURS_PER_DAY=         24        // Hours per day
,  MONTHS_PER_YEAR=       12        // Months per year
}; // enum

//----------------------------------------------------------------------------
// GregorianDate.Static attributes
//----------------------------------------------------------------------------
enum SWITCHOVER                     // The switchover date
{  SWITCH_JULIAN= 2299160           // The Julian date
,  SWITCH_MM= 10                    // The month
,  SWITCH_DJ= 4                     // The day of the month (before)
,  SWITCH_DG= 15                    // The day of the month (after)
,  SWITCH_YY= 1582                  // The year
}; // enum SWITCHOVER

static const int       dayOfYear[2][12]=
{  {   0,  31,  59,  90, 120, 151, 181, 212, 243, 273, 304, 334} // Ordinary
,  {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335} // Leap year
};

static const unsigned  daysPerMonth[2][12]=
{  {  31,  28,  31,  30,  31,  30,  31,  31,  30,  31,  30,  31} // Ordinary
,  {  31,  29,  31,  30,  31,  30,  31,  31,  30,  31,  30,  31} // Leap year
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       Calendar::~Calendar( void )
//
// Function-
//       Destructor
//
//----------------------------------------------------------------------------
   Calendar::~Calendar( void )      // Destructor
{
   #ifdef HCDM
     tracef("%8s= Calendar(%p)::~Calendar()\n", "", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Calendar::Calendar( void )
//
// Function-
//       Constructor
//
//----------------------------------------------------------------------------
   Calendar::Calendar( void )       // Constructor
:  year(1)
,  month(1)
,  day(1)
,  hour(0)
,  minute(0)
,  second(0)
,  millisecond(0)
{
   #ifdef HCDM
     tracef("%8s= Calendar(%p)::Calendar()\n", "", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Calendar::Calendar(const Calendar&)
//
// Function-
//       Copy constructor.
//
//----------------------------------------------------------------------------
   Calendar::Calendar(              // Copy constructor
     const Calendar&   source)      // Source
{
   #ifdef HCDM
     tracef("%8s= Calendar(%p)::Calendar(Calendar(%p))\n", "",
            this, &source);
   #endif

   set(source);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Calendar::Calendar(const Julian&)
//
// Function-
//       Constructor.
//
//----------------------------------------------------------------------------
   Calendar::Calendar(              // Constructor
     const Julian&     source)      // Source
{
   #ifdef HCDM
     tracef("%8s= Calendar(%p)::Calendar(Julian(%p))\n", "",
            this, &source);
   #endif

   set(source);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Calendar::Calendar(const Clock&)
//
// Function-
//       Constructor.
//
//----------------------------------------------------------------------------
   Calendar::Calendar(              // Constructor
     const Clock&      source)      // Source
{
   #ifdef HCDM
     tracef("%8s= Calendar(%p)::Calendar(Clock(%p))\n", "",
            this, &source);
   #endif

   Julian julian(source);
   set(julian);
}

//----------------------------------------------------------------------------
//
// Method-
//       Calendar::operator=
//
// Function-
//       Assign to this Calendar.
//
//----------------------------------------------------------------------------
Calendar&                           // Resultant
   Calendar::operator=(             // Assign to this
     const Calendar&   source)      // Source
{
   set(source);
   return *this;
}

Calendar&                           // Resultant
   Calendar::operator=(             // Assign to this
     const Clock&      source)      // Source
{
   Julian julian(source);
   set(julian);
   return *this;
}

Calendar&                           // Resultant
   Calendar::operator=(             // Assign to this
     const Julian&     source)      // Source
{
   set(source);
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Calendar::compare
//
// Function-
//       Compare Calendar objects.
//
//----------------------------------------------------------------------------
int                                 // Resultant (<0, 0, >0)
   Calendar::compare(               // Compare to
     const Calendar&   comprahend) const // Comprahend
{
   int                 resultant;

   if( year < comprahend.year )
     resultant= -1;
   else if( year > comprahend.year )
     resultant= +1;
   else if( month < comprahend.month )
     resultant= -1;
   else if( month > comprahend.month )
     resultant= +1;
   else if( day < comprahend.day )
     resultant= -1;
   else if( day > comprahend.day )
     resultant= +1;
   else if( hour < comprahend.hour )
     resultant= -1;
   else if( hour > comprahend.hour )
     resultant= +1;
   else if( minute < comprahend.minute )
     resultant= -1;
   else if( minute > comprahend.minute )
     resultant= +1;
   else if( second < comprahend.second )
     resultant= -1;
   else if( second > comprahend.second )
     resultant= +1;
   else if( millisecond < comprahend.millisecond )
     resultant= -1;
   else if( millisecond > comprahend.millisecond )
     resultant= +1;
   else
     resultant= 0;

   return resultant;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Calendar::set
//
// Purpose-
//       Set the Calendar to the current date.
//
//----------------------------------------------------------------------------
void
   Calendar::set( void )            // Set the Calendar to the current date
{
   Julian julian(Julian::current()); // The Julian
   *this= julian;
}

//----------------------------------------------------------------------------
//
// Method-
//       Calendar::set(const Calendar&)
//
// Function-
//       Set the Calendar.
//
//----------------------------------------------------------------------------
void
   Calendar::set(                   // Set the Calendar
     const Calendar&   source)      // Source
{
   year=   source.year;
   month=  source.month;
   day=    source.day;
   hour=   source.hour;
   minute= source.minute;
   second= source.second;
   millisecond= source.millisecond;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Calendar::set(const Julian&)
//
// Purpose-
//       Set the Calendar.
//
//----------------------------------------------------------------------------
void
   Calendar::set(                   // Set the Calendar
     const Julian&     julian)      // The source Julian
{
   int64_t             julianDate;  // The numerical Julian date
   uint64_t            julianTime;  // The number of milliseonds past 00:00

   int64_t             century;     // The Julian century
   int64_t             cycles;      // Number of complete four-year cycles
   int                 wday;        // Working day
   int64_t             wyear;       // Working year

   int                 isLeap;      // 0 if ordinary, 1 if leap year

   int                 x;

   julianDate= (int64_t)julian.getDate(); // Get the integer Julian date
   if( julianDate > SWITCH_JULIAN ) // If after switchover
   {
     century= (int64_t)((double)(julianDate-1684595) / 36524.25);
     julianDate += ((century*3)/4) - 2;
   }

   cycles= julianDate/1461;         // The number of complete 4-year cycles
   wyear= cycles*4;                 // The number of years
   wday= (int)(julianDate%1461);    // The number of days left in cycle

   if( julianDate < 0 )             // If before 01/01/(-4712)
   {
     wyear= wyear-4;
     wday= 1461 + wday;             // (wday is negative)
   }

   for(x= 0; x<4 && 365*(x+1) < wday; x++)
   {
   }

   if( x == 0 && wyear <= (4712+SWITCH_YY) )
   {
     isLeap= 1;
     wday++;
   }

   else if( x == 0 && wyear > (4712+SWITCH_YY)
            && (((wyear-4712) % 100) != 0 || ((wyear-4712) % 400) == 0) )
   {
     isLeap= 1;
     wday++;
   }

   else if( x == 0 && wyear > (4712+SWITCH_YY)
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

   year=  wyear - 4712;
   month= x-1;
   day=   wday - dayOfYear[isLeap][month-1];

   // Get the time of day
   julianDate= julian.getDate();
   double tod= julian.getTime() - double(julianDate*SECONDS_PER_DAY);
   julianTime= uint64_t(tod * MILLISECONDS_PER_SECOND * 10);
   millisecond= ((julianTime + 5) % (MILLISECONDS_PER_SECOND * 10)/10);
   julianTime= uint64_t(tod);

   second= julianTime % SECONDS_PER_MINUTE;
   julianTime /= SECONDS_PER_MINUTE;

   minute= julianTime % MINUTES_PER_HOUR;
   julianTime /= MINUTES_PER_HOUR;

   hour= julianTime;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Calendar::setYMDHMSN
//
// Purpose-
//       Set the Calendar.
//
//----------------------------------------------------------------------------
void
   Calendar::setYMDHMSN(            // Set the Calendar
     int64_t           year,        // The year
     unsigned int      month,       // The month, range 1..12
     unsigned int      day,         // The day of the month, range 1..31
     unsigned int      hour,        // The hour of the day, range 0..23
     unsigned int      minute,      // The minute of the hour, range 0..59
     unsigned int      second,      // The second of the minute, range 0..59
     uint32_t          millisecond) // The millisecond, range 0..999
{
   int                 isLeap;      // 1 if leap year, 0 otherwise

   isLeap= 0;                       // Default, not a leap year
   if( year < 1752 )
   {
     if( (year%4) == 0 )
       isLeap= 1;
   }
   else if( (year%4) == 0 && ((year%100) != 0 || (year%400) == 0) )
     isLeap= 1;

   if( month < 1 || month > MONTHS_PER_YEAR
       || day < 1 || day > daysPerMonth[isLeap][month-1]
       || hour >= HOURS_PER_DAY
       || minute >= MINUTES_PER_HOUR
       || second >= SECONDS_PER_MINUTE
       || millisecond >= MILLISECONDS_PER_SECOND
       || (year == SWITCH_YY && month == SWITCH_MM && (day > SWITCH_DJ && day < SWITCH_DG)) )
     throw "Calendar::set()RangeError";

   this->year= year;
   this->month= month;
   this->day= day;

   this->hour= hour;
   this->minute= minute;
   this->second= second;
   this->millisecond= millisecond;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Calendar::setYMD
//
// Purpose-
//       Set the Calendar.
//
//----------------------------------------------------------------------------
void
   Calendar::setYMD(                // Set the Calendar
     int64_t           year,        // The year
     unsigned int      month,       // The month, range 1..12
     unsigned int      day)         // The day of the month, range 1..31
{
   setYMDHMSN(year, month, day, 0, 0, 0, 0);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Calendar::toClock
//
// Purpose-
//       Convert to Clock
//
//----------------------------------------------------------------------------
Clock                               // The resultant Clock
   Calendar::toClock( void ) const  // Get a Clock from this Calendar
{
   Clock resultant(toJulian());
   return resultant;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Calendar::toJulian
//
// Purpose-
//       Convert to Julian
//
//----------------------------------------------------------------------------
Julian                              // The resultant Julian
   Calendar::toJulian( void ) const // Get a Julian from this Calendar
{
   int64_t             date;        // The resultant numerical day

   int64_t             century;     // Working century
   int64_t             wyear;       // Working year
   int                 leap;        // 0 if ordinary, 1 if leap year

   wyear= year + 4712;              // Day 0 is Jan 1, 4713 BCE
   if( wyear < -13343998895911LL || wyear > 13343998895911LL )
     throw "Calendar.get(Julian&)RangeException";

   // Compute the Reference date
   if( wyear < 0 )
   {
     date= 365*wyear + wyear/4 - 1;
     leap= 0;
     if( (wyear%4) == 0 )
       leap= 1;
   }
   else
   {
     date= 365*wyear + wyear/4;
     leap= 0;
     if( (wyear%4) == 0 )
     {
       leap= 1;
       date--;
     }
   }

   date += dayOfYear[leap][month-1] + day;
   if( date > SWITCH_JULIAN )       // If Gregorian date after switchover
   {
     wyear= year - 300;
     if( month < 3 )
       wyear--;

     century= wyear/100;
     date= date - ((century*3)/4) - 1;
   }

   double jsec= date * double(SECONDS_PER_DAY);
   jsec += hour * 3600.0;
   jsec += minute * 60.0;
   jsec += second;
   jsec += millisecond / double(MILLISECONDS_PER_SECOND);
   Julian resultant(jsec); // The resultant Julian
   return resultant;
}

