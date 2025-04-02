//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2025 Frank Eskesen.
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
//       Implement Calendar.h
//
// Last change date-
//       2025/04/02
//
// Implementation notes-
//       The Calendar::day2ymd table is instantiated by Cal400.cpp
//
//----------------------------------------------------------------------------
#include <stdexcept>                // For std::invalid_argument
#include <tuple>                    // For std::tuple

#include <math.h>                   // For floor, fmod, ...

#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/Calendar.h"           // For pub::Calendar, implemented
#include "pub/Clock.h"              // For pub::Clock
#include "pub/Julian.h"             // For pub::Julian
#include <pub/utility.h>            // For pub::utility::to_string

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging methods
using pub::Clock;                   // For convenience
using pub::Julian;                  // For convenience
using pub::utility::to_string;      // For convenience

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // generic enum

//----------------------------------------------------------------------------
// Typedefs and enumerations
//----------------------------------------------------------------------------
typedef int64_t        Cycle;       // Cycle number
typedef unsigned       Cycle_unit;  // Part of Cycle, range 0 .. (Cycle_size-1)
typedef unsigned       Cycle_size;  // The number of units in a Cycle
typedef int64_t        JD;          // Julian Day number
typedef int64_t        Year;        // Calendar year
typedef unsigned       Month;       // Month of Year, range 1 .. 12
typedef unsigned       Day;         // Day of Month, range 1 .. 31

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static const unsigned  day_of_year[2][13]= // First day of month
{  {   0,  31,  59,  90, 120, 151, 181, 212, 243, 273, 304, 334, 365} // Common
,  {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335, 366} // Leap
};

static const unsigned  days_per_month[2][12]= // Number of days in month
{  {  31,  28,  31,  30,  31,  30,  31,  31,  30,  31,  30,  31} // Common year
,  {  31,  29,  31,  30,  31,  30,  31,  31,  30,  31,  30,  31} // Leap year
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Julian calendar:
static constexpr const int DAYS_PER_COMMON_YEAR= 365;
static constexpr const int DAYS_PER_LEAP_YEAR= 366;

static constexpr const int JULIAN_DAY_ZERO= 1'721'058; // (1/1/0000) Julian
static constexpr const int YEARS_PER_JCYCLE= 4; // Years in a complete cycle
static constexpr const int DAYS_PER_JCYCLE= // Days in a complete  4-year cycle
     DAYS_PER_COMMON_YEAR * YEARS_PER_JCYCLE + 1; // (1'461)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Gregorian calendar:
// A Gregorian cycle contains 400 years.
// Years evenly divisible by 4 are leap years, except for years that also begin
// centuries. Century 0 is a leap year, centuries 1, 2, and 3 are not.
// (There are 100 possible leap years in 400 years. Since centuries 1, 2, and 3
// do not begin with leap years, there are actually only 97.)
// GREGORIAN_DAY_ZERO (1/1/0000 Gregorian) is 1/3/0000 in the Julian calendar.

static constexpr const int GREGORIAN_DAY_ZERO= 1'721'060;
static constexpr const int YEARS_PER_GCYCLE= 400; // Years in a complete cycle
static constexpr const int DAYS_PER_GCYCLE= // Days in a complete cycle
     (DAYS_PER_COMMON_YEAR * YEARS_PER_GCYCLE) + 97; // 146,097

//----------------------------------------------------------------------------
//
// Subroutine-
//       bisect
//
// Purpose-
//       Get Cycle number and Cycle offset given JD and Cycle_size
//
// Implementation notes-
//       The JD group can be in any unit, but we only use days or years.
//
//----------------------------------------------------------------------------
static std::tuple<Cycle, Cycle_unit> // The Cycle and the cycle offset
   bisect(                          // Get Cycle and the cycle offset
     JD                group,       // For this combined group of Days or Years
     Cycle_size        size)        // And this Cycle_size, in days or years
{
   Cycle cycle= group >= 0 ? group / size : (group - size + 1) / size;
   JD    floor= cycle * size;
   return {cycle, group - floor};
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_Gregorian
//
// Function-
//       Determine whether a specified date is in the Gregorian calendar.
//
//----------------------------------------------------------------------------
static bool                         // TRUE if the current day is Gregorian
   _is_Gregorian(                   // Is the specified date Gregorian
     Year              year,        // The specified Year
     Month             month,       // The specified Month
     Day               day)         // The specified Day
{
   if( year  < Calendar::SWITCH_YY )
     return false;

   if( year  > Calendar::SWITCH_YY )
     return true;

   if( month < Calendar::SWITCH_MM )
     return false;

   if( month > Calendar::SWITCH_MM )
     return true;

   if( day   < Calendar::SWITCH_DG )
     return false;

   return true;
}

static inline bool                  // TRUE if the current day is Gregorian
   is_Gregorian(                    // Is the specified date Gregorian
     Year              year,        // The specified Year
     Month             month,       // The specified Month
     Day               day)         // The specified Day
{
   bool B= _is_Gregorian(year, month, day);
   if( HCDM )
     debugf("%s= is_Gregorian(%zd,%d,%d)\n", B ? "true" : "false"
           , year, month, day);
   return B;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       leap_ix_g
//
// Function-
//       Get Gregorian calendar leap year index: 1 if leap year, 0 otherwise
//
//----------------------------------------------------------------------------
static inline int                   // The leap year index
   leap_ix_g(                       // Get Gregorian calendar leap year index
     Year              year)        // For this year
{
   int is_leap= int((year % 4) == 0); // Default, Julian leap year index

   // Gregorian first year of century adjustment
   if( is_leap && (year % 100) == 0 && (year % 400) != 0)
     is_leap= 0;

   return is_leap;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       leap_ix_j
//
// Function-
//       Get Julian calendar leap year index: 1 if leap year, 0 otherwise
//
//----------------------------------------------------------------------------
static inline int                   // The leap year index
   leap_ix_j(                       // Get Julian calendar leap year index
     Year              year)        // For this year
{  return int((year % 4) == 0); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       leap_ix
//
// Function-
//       Get Julian/Gregorian leap year index
//
//----------------------------------------------------------------------------
static inline int                   // The leap year index
   _leap_ix(                        // Get leap year index
     Year              year)        // For this year
{
   if( year > Calendar::SWITCH_YY ) // (The switch year is a common year)
     return leap_ix_g(year);
   else
     return leap_ix_j(year);
}

static inline int                   // The leap year index
   leap_ix(                         // Get leap year index
     Year              year)        // For this year
{
   int is_leap= _leap_ix(year);
   if( HCDM )
     debugf("%d= leap_ix(%zd)\n", is_leap, year);
   return is_leap;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_day_of_year
//
// Function-
//       Get the day of the year
//
//----------------------------------------------------------------------------
static Day                          // Day, range 0..365
   _get_day_of_year(                // Get day of the year for
     int               is_leap,     // If year is a leap year ? 1 : 0
     Month             month,       // This month (1 origin) and
     Day               day)         // This day of the month (1 origin)
{  return day_of_year[is_leap][month - 1] + day - 1; }

static Day                          // Day, range 0..365
   get_day_of_year(                 // Get day of the year for
     int               is_leap,     // If year is a leap year ? 1 : 0
     Month             month,       // This month (1 origin) and
     Day               day)         // This day of the month (1 origin)
{
   Day doy= _get_day_of_year(is_leap, month, day);
   if( HCDM )
     debugf("%d= get_day_of_year(%d,%d,%d)\n"
          , doy, is_leap, month, day);
   return doy;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       julian2ymd_g
//
// Function-
//       Convert a Julian day to a GREGORIAN year, month, and day
//
//----------------------------------------------------------------------------
static std::tuple<Year, Month, Day> // Year, Month, Day
   _julian2ymd_g(                   // Get year, month, and day
     JD                julian)      // From this GREGORIAN calendar Julian Day
{
   JD origin= julian - GREGORIAN_DAY_ZERO;
   auto [cycle, dayof]= bisect(origin, DAYS_PER_GCYCLE);

   Calendar::YMD ymd= Calendar::day2ymd[dayof];
   Year  year= cycle * YEARS_PER_GCYCLE + ymd.year;
   Month month= ymd.month;
   Day   day= ymd.day;

   return {year, month, day};
}

static std::tuple<Year, Month, Day> // Year, Month, Day
   julian2ymd_g(                    // Get year, month, and day
     JD                julian)      // From this GREGORIAN calendar Julian Day
{
   auto [year, month, day]= _julian2ymd_g(julian);
   if( HCDM )
     debugf("{%'zd, %d, %d}= julian2ymd_g(%zd)\n", year, month, day, julian);
   return {year, month, day};
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       julian2ymd_j
//
// Function-
//       Convert a Julian day to a JULIAN year, month, and day
//
//----------------------------------------------------------------------------
static std::tuple<Year, Month, Day> // Year, Month, Day
   _julian2ymd_j(                   // Get year, month, and day
     JD                julian)      // From this JULIAN calendar Julian Day
{
   JD origin= julian - JULIAN_DAY_ZERO;
   auto [cycle, dayof]= bisect(origin, DAYS_PER_JCYCLE);

   Calendar::YMD ymd= Calendar::day2ymd[dayof];
   Year  year= cycle * YEARS_PER_JCYCLE + ymd.year;
   Month month= ymd.month;
   Day   day= ymd.day;

   return {year, month, day};
}

static std::tuple<Year, Month, Day> // Year, Month, Day
   julian2ymd_j(                    // Get year, month, and day
     JD                julian)      // From this JULIAN calendar Julian Day
{
   auto [year, month, day]= _julian2ymd_j(julian);
   if( HCDM )
     debugf("{%'zd, %d, %d}= julian2ymd_j(%zd)\n", year, month, day, julian);
   return {year, month, day};
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ymd2julian_g
//
// Function-
//       Convert a GREGORIAN calendar {year, month, day} into a Julian day
//
//----------------------------------------------------------------------------
// Forward reference
static JD                           // The Julian calendar Julian day
   _ymd2julian_j(                   // Convert into Julian day
     Year              year,        // This year
     Month             month,       // This month (1 origin)
     Day               day);          // This day of the month (1 origin)

static JD                           // The GREGORIAN calendar Julian day
   _ymd2julian_g(                   // Convert into Julian day
     Year              year,        // This GREGORIAN year (0 origin)
     Month             month,       // This month (1 origin)
     Day               day)         // This day of the month (1 origin)
{
   auto [cycle, yearof]= bisect(year, YEARS_PER_GCYCLE);

   // Start with the Julian calendar date starting from the GCYCLE origin
   JD julian= _ymd2julian_j(yearof, month, day)
            - JULIAN_DAY_ZERO;      // (Subtracting out Julian day 0)

   // Correct for missing leap days
   if( julian > 109'633 )           // Year(300), Month(2), Day(28) + 2
     julian -= 3;                   // Account for 3 missing leap days
   else if( julian > 73'108)        // Year(200), Month(2), Day(28) + 1
     julian -= 2;                   // Account for 2 missing leap days
   else if( julian > 36'583)        // Year(100), Month(2), Day(28) + 0
     --julian;                      // Account for 1 missing leap day

   // Account for the completed Gregorian cycles and the Gregorian origin
   julian += GREGORIAN_DAY_ZERO + cycle * DAYS_PER_GCYCLE;

   return julian;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static JD                           // The GREGORIAN calendar Julian day
   ymd2julian_g(                    // Convert into Julian day
     Year              year,        // This year
     Month             month,       // This month (1 origin)
     Day               day)         // This day of the month (1 origin)
{
   JD julian= _ymd2julian_g(year, month, day);
   if( HCDM )
     debugf("%'zd= ymd2julian_g(%zd,%d,%d)\n", julian, year, month, day);
   return julian;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ymd2julian_j
//
// Function-
//       Convert a JULIAN {Year, Month, Day} into a Julian day
//
//----------------------------------------------------------------------------
static JD                           // The Julian calendar Julian day
   _ymd2julian_j(                   // Convert into Julian day
     Year              year,        // This year
     Month             month,       // This month (1 origin)
     Day               day)         // This day of the month (1 origin)
{
   auto [cycle, yearof]= bisect(year, YEARS_PER_JCYCLE);

   JD julian= JULIAN_DAY_ZERO
            + cycle * DAYS_PER_JCYCLE
            + yearof * DAYS_PER_COMMON_YEAR
            + get_day_of_year(leap_ix_j(yearof), month, day);
   if( yearof > 0 )                 // If not the first year of a cycle
     ++julian;                      // Account for the first year's leap day

   return julian;
}

static JD                           // The GREGORIAN calendar Julian day
   ymd2julian_j(                    // Convert into Julian day
     Year              year,        // This year
     Month             month,       // This month (1 origin)
     Day               day)         // This day of the month (1 origin)
{
   JD julian= _ymd2julian_j(year, month, day);
   if( HCDM )
     debugf("%'zd= ymd2julian_j(%zd,%d,%d)\n", julian, year, month, day);
   return julian;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ymd2julian
//
// Function-
//       Convert {year, month, day} into Gregorian or Julian day
//
//----------------------------------------------------------------------------
static JD                           // The Julian Day
   ymd2julian(                      // Convert year, month, and day into a JD
     Year              year,        // This year
     Month             month,       // This month (1 origin)
     Day               day)         // This day of the month (1 origin)
{
   if( is_Gregorian(year, month, day ) )
     return ymd2julian_g(year, month, day);
   else
     return ymd2julian_j(year, month, day);
}

//============================================================================
//
// Method-
//       Calendar::operator Clock
//
// Purpose-
//       Cast to Clock
//
//----------------------------------------------------------------------------
   Calendar::operator Clock( void ) const // Cast to Clock
{
   Julian julian= operator Julian(); // Convert to Julian
   julian += Julian::UTC1970;       // Convert to Clock origin

   Clock clock((double)julian * Julian::SECONDS_PER_DAY);
   return clock;
}

//----------------------------------------------------------------------------
//
// Method-
//       Calendar::operator Julian
//
// Purpose-
//       Cast to Julian
//
//----------------------------------------------------------------------------
   Calendar::operator Julian( void ) const // Cast to Julian
{  if( HCDM )
     debugf("operator Julian(%s)\n", ((std::string)*this).c_str());

   double julian= hour * 3600.0;
   julian += minute * 60.0;
   julian += second;
   julian /= SECONDS_PER_DAY;
   julian += (double)ymd2julian(year, month, day);

   return Julian(julian);
}

//----------------------------------------------------------------------------
//
// Method-
//       Calendar::operator std::string
//
// Purpose-
//       Cast to std::string
//
//----------------------------------------------------------------------------
   Calendar::operator std::string( void ) const // Cast to std::string
{
   return to_string("%.2d/%.2d/%.4zd %.2d:%.2d:%09.6f"
                   , month, day, year, hour, minute, second);
}

//----------------------------------------------------------------------------
//
// Method-
//       Calendar::operator=
//
// Purpose-
//       Assignment operators
//
//----------------------------------------------------------------------------
Calendar&                           // Resultant
   Calendar::operator=(             // Assignment operator
     const Calendar&   calendar)    // Source Calendar
{  set(calendar); return *this; }   // Copy source Calendar

Calendar&                           // Resultant
   Calendar::operator=(             // Assignment operator
     const Clock&      clock)       // Source Clock
{  set(clock); return *this; }      // Convert Clock to Calendar

Calendar&                           // Resultant
   Calendar::operator=(             // Assignment operator
     const Julian&     julian)      // Source Julian
{  set(julian); return *this; }     // Convert Julian to Calendar

//----------------------------------------------------------------------------
//
// Method-
//       Calendar::setYMDHMS(...)
//
// Purpose-
//       Explicitly set the Calendar.
//
//----------------------------------------------------------------------------
void
   Calendar::setYMDHMS(             // Set the Calendar
     int64_t           year,        // The year
     unsigned int      month,       // The month, range 1..12
     unsigned int      day,         // The day of the month, range 1..31
     unsigned int      hour,        // The hour of the day, range 0..23
     unsigned int      minute,      // The minute of the hour, range 0..59
     double            second)      // The second of the minute, range 0..59
{  if( HCDM )
     debugf("Calendar.setYMDHMS(%.4zd,%'.2d,%.2d %.2d,%.2d,%09.6f)\n"
     , year, month, day, hour, minute, second);

   if( month < 1 || month > MONTHS_PER_YEAR )
     throw std::invalid_argument("Invalid month");

   if( day < 1 || day > days_per_month[leap_ix(year)][month-1] )
     throw std::invalid_argument("Invalid day of month");

   if( hour >= HOURS_PER_DAY )
     throw std::invalid_argument("Invalid hour");

   if( minute >= MINUTES_PER_HOUR )
     throw std::invalid_argument("Invalid minute");

   if( second < 0.0 || second >= SECONDS_PER_MINUTE )
     throw std::invalid_argument("Invalid second");

   if( year == SWITCH_YY && month == SWITCH_MM
       && (day > SWITCH_DJ && day < SWITCH_DG) )
     throw std::invalid_argument("Nonexistent day of month");

   this->year= year;
   this->month= month;
   this->day= day;

   this->hour= hour;
   this->minute= minute;
   this->second= second;
}

//----------------------------------------------------------------------------
//
// Method-
//       Calendar::set(void)
//       Calendar::set(const Calendar&)
//       Calendar::set(const Clock&)
//       Calendar::set(const Julian&)
//
// Purpose-
//       Set the Calendar to the current date and time.
//       Set the Calendar from another Calendar. (A copy operation)
//       Set the Calendar from a Clock.
//       Set the Calendar from a Julian.
//
//----------------------------------------------------------------------------
void
   Calendar::set( void )            // Set the Calendar to current date/time
{  Julian julian; set(julian); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   Calendar::set(                   // Set the Calendar
     const Calendar&   calendar)    // From this Calendar
{
   year=   calendar.year;
   month=  calendar.month;
   day=    calendar.day;
   hour=   calendar.hour;
   minute= calendar.minute;
   second= calendar.second;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   Calendar::set(                   // Set the Calendar
     const Clock&      clock)       // From this Clock
{  Julian julian(clock); set(julian); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   Calendar::set(                   // Set the Calendar
     const Julian&     julian)      // From this Julian
{
   double ddd= floor(julian.get()); // The associated Julian day
   if( HCDM )
     debugf("Calendar.set(Julian(%'.6f))\n", ddd);

   if( ddd >= SWITCH_JD ) {         // If Gregorian calendar date
     auto [year, month, day]= julian2ymd_g(int64_t(ddd));
     this->year= year;
     this->month= month;
     this->day= day;
   } else {                         // If Julian calendar date
     auto [year, month, day]= julian2ymd_j(int64_t(ddd));
     this->year= year;
     this->month= month;
     this->day= day;
   }

   // Handle time of day
   double tod= julian.get_tod();
   double second= fmod(tod, SECONDS_PER_MINUTE); // Fractional second
   int hhmm= int(tod / SECONDS_PER_MINUTE);
   this->hour=   hhmm / MINUTES_PER_HOUR;
   this->minute= hhmm % MINUTES_PER_HOUR;
   this->second= second;
}
} // namespace _LIBPUB_NAMESPACE
