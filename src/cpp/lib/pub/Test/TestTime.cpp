//----------------------------------------------------------------------------
//
//       Copyright (C) 2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       TestTime.cpp
//
// Purpose-
//       Test time functions: Calendar, Clock, Julian
//
// Last change date-
//       2025/04/29
//
//----------------------------------------------------------------------------
#include <clocale>                  // For setlocale
#include <cstdlib>                  // For size_t, rand, srand
#include <cstring>                  // For memcmp, ...
#include <ctime>                    // For time

#include <math.h>                   // For floor

#include <pub/TEST.H>               // For VERIFY macros
#include "pub/Calendar.h"           // For pub::Calendar, tested
#include "pub/Clock.h"              // For pub::Clock, tested
#include <pub/Debug.h>              // For pub::Debug, namespace pub::debugging
#include "pub/Julian.h"             // For pub::Julian, tested
#include <pub/Reporter.h>           // For pub::Reporter
#include <pub/utility.h>            // For pub::utilities
#include <pub/utility.i>            // For pub::utility conversion routines
#include <pub/Wrapper.h>            // For pub::Wrapper

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For convenience
using namespace PUB::debugging;     // For debugging methods
using PUB::utility::visify;         // For convenience (defined in utility.h)
using PUB::s2c;                     // For convenience (defined in utility.i)
using std::string;                  // For convenience

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

static const double    MICRODAY= 0.000001; // Microday precison
static const double    SPD= 86'400; // Seconds Per Day

// Verification (Julian) date ranges
static double          MIN_GREGOR= 2458850; // 1/1/2020
static double          MAX_GREGOR= 2462503; // 1/1/2030
static double          MIN_JULIAN= -105192; // 1/1/-5000 Julian
static double          MAX_JULIAN= 3547273; // 1/1/+5000 Gregorian

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
// Extended options
static int             opt_dirty= false; // --dirty
static int             opt_display= false;  // --display
static int             opt_size= false;  // --size
static int             opt_calendar= false; // --calendar
static int             opt_clock= false;    // --clock
static int             opt_julian= false;   // --julian
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"dirty",    no_argument,       &opt_dirty,      true} // --dirty
,  {"display",  no_argument,       &opt_display,    true} // --display
,  {"size",     no_argument,       &opt_size,       true} // --size
,  {"calendar", no_argument,       &opt_calendar,   true} // --calendar
,  {"clock",    no_argument,       &opt_clock,      true} // --clock
,  {"julian",   no_argument,       &opt_julian,     true} // --julian
,  {"all",      optional_argument, nullptr,            0} // --all
,  {0, 0, 0, 0}                     // (End of option list)
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       size_of
//
// Purpose-
//       Display size of something
//
//----------------------------------------------------------------------------
#define SIZEOF(x) size_of(sizeof(x), #x)

static inline void
   size_of(                         // Display size of something
     size_t            size,        // The size
     const char*       name)        // The something's name
{  debugf("%'8zd= sizeof(%s)\n", size, name); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       SNO
//
// Purpose-
//       Should Not Occur condition occurred
//
//----------------------------------------------------------------------------
static inline void
   SNO(int line)                    // Should Not Occur
{  debugf("%4d %s SHOULD NOT OCCUR\n", line, __FILE__);
   throw std::runtime_error("should not occur");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       D2C
//
// Purpose-
//       Convert double to Clock
//
//----------------------------------------------------------------------------

static inline Clock                 // Associated Clock
   D2C(                             // Convert Julian date to Clock
     double            date)        // The Clock time
{  Clock clock(date);  return clock; }

//----------------------------------------------------------------------------
//
// Subroutine-
//       verify_day
//
// Purpose-
//       Verify Julian and Calendar/Clock consistency
//
// Implementation notes:
//       Checks: Julian=>Calendar=>Julian
//       Checks: Clock=>Julian=>Clock
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE if values are (approximately) equal
   verify_day(                      // Verify
     const Julian&     rhj,         // This Julian date
     double            precision,   // To this precision
     int               verbosity= opt_verbose)
{
   bool                OK= true;    // (So far, so good)

   // Julian=>Calendar=>Julian
   Calendar rhy(rhj);               // Julian=>Calendar
   Clock    rhc(rhj);               // Julian=>Clock
   Julian   lhj(rhy);               // Julian=>Calendar=>Julian

   if( fabs((double)lhj - (double)rhj) > precision ) {
     debugf("verify_day NG: %02d/%02d/%04zd%s  %'18.8f  %'23.6f\n"
           , rhy.get_month(), rhy.get_day(), rhy.get_year()
           , rhy.get_year() >= -999 ? " " : ""
           , rhj.get(), rhc.get());
     debugf("Julian: %'14.6f != %'14.6f; Julian=>Calendar=>Julian\n"
           , (double)lhj, (double)rhj);
     OK= false;
   }

   // Clock=>Julian=>Clock
   lhj=     rhc;                    // (Julian=>)Clock=>Julian
   Clock    lhc(lhj);               // (Julian=>)Clock=>Julian=>Clock

   if( fabs((double)lhc - (double)rhc) > precision ) {
     debugf("verify_day NG: %02d/%02d/%04zd%s  %'18.8f  %'23.6f\n"
           , rhy.get_month(), rhy.get_day(), rhy.get_year()
           , rhy.get_year() >= -999 ? " " : ""
           , rhj.get(), rhc.get());
     debugf("Clock: %'14.6f != %'14.6f; Julian=>Clock=>Julian=>Clock\n"
           , (double)lhc, (double)rhc);
     OK= false;
   }

   if( OK && (verbosity > 1) ) {
     debugf("verify_day OK: %02d/%02d/%04zd%s  %'18.8f  %'23.6f\n"
           , rhy.get_month(), rhy.get_day(), rhy.get_year()
           , rhy.get_year() >= -999 ? " " : ""
           , lhj.get(), lhc.get());
   }

   return OK;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verify_tod
//
// Purpose-
//       Verify Clock and Calendar/Julian consistency
//
// Implementation notes:
//       Checks: Clock=>Calendar=>Clock
//       Checks: Julian=>Clock=>Julian
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE if values are (approximately) equal
   verify_tod(                      // Verify
     const Clock&      rhc,         // This Clock time
     double            precision,   // To this precision
     int               verbosity= opt_verbose)
{
   bool                OK= true;    // (So far, so good)

   // Clock=>Calendar=>Clock
   Calendar rhy(rhc);               // Clock=>Calendar
   Julian   rhj(rhc);               // Clock=>Julian
   Clock    lhc= (Clock)rhy;        // Clock=>Calendar=>Clock

   if( abs((double)lhc - (double)rhc) > precision ) {
     debugf("verify_tod NG: %02d/%02d/%04zd%s  %'18.8f  %'23.6f\n"
           , rhy.get_month(), rhy.get_day(), rhy.get_year()
           , rhy.get_year() >= -999 ? " " : ""
           , rhj.get(), rhc.get());
     debugf("Clock: %'14.8f != %'14.8f; Clock=>Calendar=>Clock\n"
           , (double)lhc, (double)rhc);
     debugf("Clock: %'14.8f != %'14.8f; Clock=>Calendar=>Clock\n"
           , (double)lhc, (double)rhc);
     OK= false;
   }

   // (Clock=>)Julian=>Clock=>Julian
   lhc= (Clock)rhj;                 // (Clock=>)Julian=>Clock
   Julian   lhj(lhc);               // (Clock=>)Julian=>Clock=>Julian
   if( abs((double)lhj - (double)rhj) > precision ) {
     debugf("verify_tod NG: %02d/%02d/%04zd%s  %'18.8f  %'23.6f\n"
           , rhy.get_month(), rhy.get_day(), rhy.get_year()
           , rhy.get_year() >= -999 ? " " : ""
           , rhj.get(), rhc.get());
     debugf("Julian: %'14.8f != %'14.8f; Clock=>Julian=>Clock=>Julian\n"
           , (double)lhj, (double)rhj);
     OK= false;
   }

   if( OK && (verbosity > 1) ) {
     debugf("verify_tod OK: %02d/%02d/%04zd%s  %'18.8f  %'23.6f\n"
           , rhy.get_month(), rhy.get_day(), rhy.get_year()
           , rhy.get_year() >= -999 ? " " : ""
           , (double)rhj, (double)rhc);
   }

   // Why does this work? (Converts Calendar=>Julian=>Clock)
   Clock foo(rhy);

   return OK;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_calendar
//
// Purpose-
//       Test Calendar.h
//
//----------------------------------------------------------------------------
static inline int
   test_calendar( void )            // Test Calendar.h
{
   if( opt_verbose )
     debugf("\ntest_calendar:\n");

   int error_count= 0;

   int verbosity= opt_verbose;
   if( verbosity > 1 || opt_display ) {
     debugf("\nVerifying Calendar day:\n");
     debugf("               --- year --  --- Julian date --"
            "  ------ Clock time -----\n");
   }

   for(int64_t day= MIN_JULIAN; day <MAX_JULIAN; ++day) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( day < (MIN_JULIAN + 15) ) v_verbosity= 2;
       if( day >= MIN_GREGOR && day <= MAX_GREGOR ) {
         if( day <= (MIN_GREGOR + 15) ) v_verbosity= 2;
         if( day >= (MAX_GREGOR - 15) ) v_verbosity= 2;
       }
       if( day > (MAX_JULIAN - 15) ) v_verbosity= 2;
     }
     if( v_verbosity > 1 && (day == MIN_GREGOR
                          || day == (MAX_GREGOR - 15)) )
       debugf("\n");

     error_count += VERIFY( verify_day(Julian(day), MICRODAY, v_verbosity) );
     if( error_count > 16 )         // (Broken code escape)
       break;

     if( v_verbosity > 1 && day == MAX_GREGOR )
       debugf("\n");
   }

   //-------------------------------------------------------------------------
   // Verify the Julian time of day 0.0 .. 1.0
   if( verbosity > 1 || opt_display ) {
     debugf("\nVerifying Julian time of day range: (micro-day precision)\n");
     debugf("               --- year --  --- Julian date --"
            "  ------ Clock time -----\n");
   }

   // Test one hundred times per second with sub-microday resolution
   // A micro-day is 0.0864 seconds. (86'400 / 1'000'000)
   double increment= 0.010'000'123'456'789;

   // MIN-JULIAN
   for(double tod= increment; tod < 1.0; tod += increment) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( tod <= 0.0 + increment * 15 ) v_verbosity= 2;
       if( tod >= 1.0 - increment * 15 ) v_verbosity= 2;
     }

     error_count += VERIFY( verify_day(Julian(MIN_JULIAN - tod), MICRODAY
                                      , v_verbosity) );
     if( error_count > 16 )         // (Broken code escape)
       break;
   }

   for(double tod= 0.0; tod < 1.0; tod += increment) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( tod <= 0.0 + increment * 15 ) v_verbosity= 2;
       if( tod >= 1.0 - increment * 15 ) v_verbosity= 2;
     }

     error_count += VERIFY( verify_day(Julian(MIN_JULIAN + tod), MICRODAY
                                      , v_verbosity) );
     if( error_count > 16 )         // (Broken code escape)
       break;
   }

   //-------------------------------------------------------------------------
   // MIN-GREGORIAN
   if( verbosity > 1 || opt_display ) {
     debugf("\n");
     debugf("               --- year --  --- Julian date --"
            "  ------ Clock time -----\n");
   }

   for(double tod= 0.0; tod < 1.0; tod += increment) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( tod <= 0.0 + increment * 15 ) v_verbosity= 2;
       if( tod >= 1.0 - increment * 15 ) v_verbosity= 2;
     }

     error_count += VERIFY( verify_day(Julian(MIN_GREGOR + tod), MICRODAY
                                      , v_verbosity) );
     if( error_count > 16 )         // (Broken code escape)
       break;
   }

   //-------------------------------------------------------------------------
   // MAX-GREGORIAN
   if( verbosity > 1 || opt_display ) {
     debugf("\n");
     debugf("               --- year --  --- Julian date --"
            "  ------ Clock time -----\n");
   }

   for(double tod= 0.0; tod < 1.0; tod += increment) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( tod <= 0.0 + increment * 15 ) v_verbosity= 2;
       if( tod >= 1.0 - increment * 15 ) v_verbosity= 2;
     }

     error_count += VERIFY( verify_day(Julian(MAX_GREGOR + tod), MICRODAY
                                      , v_verbosity) );
     if( error_count > 16 )         // (Broken code escape)
       break;
   }

   //-------------------------------------------------------------------------
   // MAX-JULIAN
   if( verbosity > 1 || opt_display ) {
     debugf("\n");
     debugf("               --- year --   -- Julian date --"
            "  ------ Clock time -----\n");
   }
   for(double tod= increment; tod < 1.0; tod += increment) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( tod <= 0.0 + increment * 15 ) v_verbosity= 2;
       if( tod >= 1.0 - increment * 15 ) v_verbosity= 2;
     }

     error_count += VERIFY( verify_day(Julian(MAX_JULIAN - tod), MICRODAY
                                      , v_verbosity) );
     if( error_count > 16 )         // (Broken code escape)
       break;
   }

   for(double tod= 0.0; tod < 1.0; tod += increment) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( tod <= 0.0 + increment * 15 ) v_verbosity= 2;
       if( tod >= 1.0 - increment * 15 ) v_verbosity= 2;
     }

     error_count += VERIFY( verify_day(Julian(MAX_JULIAN + tod), MICRODAY
                                      , v_verbosity) );
     if( error_count > 16 )         // (Broken code escape)
       break;
   }

   //-------------------------------------------------------------------------
   // Verify the Clock time of day 0.0 .. 86'400.0
   double PRECISION= 0.000'100;
          increment= 1.000'123'456'789;

   if( verbosity > 1 || opt_display ) {
     debugf("\nVerifying Clock time of day range: (precision %.6f second)\n"
           , PRECISION);
     debugf("               --- year --  --- Julian date --"
            "  ------ Clock time -----\n");
   }

   Julian julian(MIN_GREGOR);
   Clock  clock= (Clock)julian;
   double day= (double)clock;
   for(double tod= 0.0; tod < SPD; tod += increment) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( tod <= 0.0 + increment * 15 ) v_verbosity= 2;
       if( tod >= SPD - increment * 15 ) v_verbosity= 2;
     }

     error_count += VERIFY( verify_tod(D2C(day-SPD+tod), PRECISION
                                      , v_verbosity) );
     if( error_count > 16 )         // (Broken code escape)
       break;
   }

   for(double tod= 0.0; tod < SPD; tod += increment) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( tod <= 0.0 + increment * 15 ) v_verbosity= 2;
       if( tod >= SPD - increment * 15 ) v_verbosity= 2;
     }
     error_count += VERIFY( verify_tod(D2C(day+tod), PRECISION, v_verbosity) );
     if( error_count > 16 )         // (Broken code escape)
       break;
   }

   if( verbosity > 1 || opt_display ) {
     debugf("\n");
     debugf("               --- year --  --- Julian date --"
            "  ------ Clock time -----\n");
   }

   julian= MAX_GREGOR;
   clock=  (Clock)julian;
   day= (double)clock;
   for(double tod= 0.0; tod < SPD; tod += increment) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( tod <= 0.0 + increment * 15 ) v_verbosity= 2;
       if( tod >= SPD - increment * 15 ) v_verbosity= 2;
     }
     error_count += VERIFY( verify_tod(D2C(day-SPD+tod), PRECISION
                                      , v_verbosity) );
     if( error_count > 16 )         // (Broken code escape)
       break;
   }

   for(double tod= 0.0; tod < SPD; tod += increment) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( tod <= 0.0 + increment * 15 ) v_verbosity= 2;
       if( tod >= SPD - increment * 15 ) v_verbosity= 2;
     }
     error_count += VERIFY( verify_tod(D2C(day+tod), PRECISION
                          , v_verbosity) );
     if( error_count > 16 )         // (Broken code escape)
       break;
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_clock
//
// Purpose-
//       Test Clock.h
//
//----------------------------------------------------------------------------
static inline int
   test_clock( void )               // Test Clock.h
{
   if( opt_verbose )
     debugf("\ntest_clock:\n");

   int error_count= 0;

   Clock L;
   Clock R= L;
   error_count += VERIFY( R == L );
   error_count += VERIFY( R <= L && L <= R );
   error_count += VERIFY( R >= L && L >= R );
   error_count += VERIFY( !(R != L) );
   error_count += VERIFY( !( R < L || L > R ) );

   L= 101;
   L -= 1;
   R= 100;
   error_count += VERIFY( L == R );
   error_count += VERIFY( L <= R );
   error_count += VERIFY( L >= R );
   error_count += VERIFY( !(L != R) );
   error_count += VERIFY( !(L <  R) );
   error_count += VERIFY( !(L >  R) );

   L= 101;
   R= 100;
   R += 1;
   error_count += VERIFY( L == R );
   error_count += VERIFY( L <= R );
   error_count += VERIFY( L >= R );
   error_count += VERIFY( !(L != R) );
   error_count += VERIFY( !(L <  R) );
   error_count += VERIFY( !(L >  R) );

   Clock LR= L + R;
   error_count += VERIFY( LR.get() == 202 );
   LR= L - R;
   error_count += VERIFY( LR.get() == 0 );

   L= 100.000000001; R= 0.000000002; LR= L -= R;
   error_count += VERIFY( LR.get() != 99 );
   error_count += VERIFY( int(LR.get()) == 99 );

   L= -10.5;
   error_count += VERIFY( int(floor(L.get())) == (-11) );

   L= +10.25;
   error_count += VERIFY( int(floor(L.get())) == (+10) );

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_julian
//
// Purpose-
//       Test Julian.h
//
//----------------------------------------------------------------------------
static inline int
   test_julian( void )              // Test Julian.h
{
   if( opt_verbose )
     debugf("\ntest_julian:\n");

   int error_count= 0;

   Julian L;
   Julian R= L;
   error_count += VERIFY( R == L );
   error_count += VERIFY( R <= L && L <= R );
   error_count += VERIFY( R >= L && L >= R );
   error_count += VERIFY( !(R != L) );
   error_count += VERIFY( !( R < L || L > R ) );

   L= 101;
   L -= 1;
   R= 100;
   error_count += VERIFY( L == R );
   error_count += VERIFY( L <= R );
   error_count += VERIFY( L >= R );
   error_count += VERIFY( !(L != R) );
   error_count += VERIFY( !(L <  R) );
   error_count += VERIFY( !(L >  R) );

   L= 101;
   R= 100;
   R += 1;
   error_count += VERIFY( L == R );
   error_count += VERIFY( L <= R );
   error_count += VERIFY( L >= R );
   error_count += VERIFY( !(L != R) );
   error_count += VERIFY( !(L <  R) );
   error_count += VERIFY( !(L >  R) );

   Julian LR= L + R;
   error_count += VERIFY( LR.get() == 202 );
   LR= L - R;
   error_count += VERIFY( LR.get() == 0 );

   L= 100.000000001; R= 0.000000002; LR= L -= R;
   error_count += VERIFY( LR.get() != 99 );
   error_count += VERIFY( int(LR.get()) == 99 );

   L= -10.5;
   error_count += VERIFY( int(floor(L.get())) == (-11) );

   L= +10.25;
   error_count += VERIFY( int(floor(L.get())) == (+10) );

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_case
//
// Purpose-
//       Sample testcase
//
//----------------------------------------------------------------------------
static inline int
   test_case( void )                // Test case
{
   if( opt_verbose )
     debugf("\ntest_case:\n");

   int error_count= 0;

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_dirty
//
// Purpose-
//       The world-famous quick and dirty test.
//
//----------------------------------------------------------------------------
static inline int
   test_dirty( void )
{

   int was_verbose= opt_verbose;
   opt_verbose= 2;
   debugf("\ntest_dirty:\n");

   int error_count= 0;

   //-------------------------------------------------------------------------
   // Special dates
   if( true ) {
     Julian   julian(0.0);
     Calendar calendar(julian);

     calendar.setYMDHMS(-5000,  1, 1); // Julian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS(-4000,  1, 1); // Julian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS(   -1,  1, 1); // Julian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS(    0,  1, 1); // Julian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS(    1,  1, 1); // Julian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS( 1000,  1, 1); // Julian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS( 1600,  1, 1); // Gregorian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS( 1700,  1, 1); // Gregorian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS( 1800,  2, 1); // Gregorian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS( 1900,  1, 1); // Gregorian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS( 2000,  1, 1); // Gregorian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS( 3000,  1, 1); // Gregorian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS( 4000,  1, 1); // Gregorian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS( 9999, 12, 31); // Gregorian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS(10000,  1, 1); // Gregorian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);
   }

   if( true ) {
     Julian   julian;
     Calendar calendar;

     calendar.setYMDHMS(-5000,  1, 1); // Julian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS(-4000,  1, 1); // Julian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS(   -1,  1, 1); // Julian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS(    0,  1, 1); // Julian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS(    1,  1, 1); // Julian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS( 1000,  1, 1); // Julian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS( 1600,  1, 1); // Gregorian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS( 2000,  1, 1); // Gregorian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS( 3000,  1, 1); // Gregorian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS( 4000,  1, 1); // Gregorian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS( 9999, 12, 31); // Gregorian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);

     calendar.setYMDHMS(10000,  1, 1); // Gregorian
     julian= (Julian)calendar;
     verify_day(julian, MICRODAY);
   }

   opt_verbose= was_verbose;

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_size
//
// Purpose-
//       Display class and structure size
//
//----------------------------------------------------------------------------
static inline int
   test_size( void )                // Test object sizes
{
   if( opt_verbose )
     debugf("\ntest_sizes:\n");

   SIZEOF(Calendar);
   SIZEOF(Clock);
   SIZEOF(Julian);

   int error_count= 0;

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   Wrapper  tc= opts;               // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   setlocale(LC_NUMERIC, "");       // Enables printf("%'d\n", 123'456'789)

   tc.on_info([]() {
     fprintf(stderr, "  --all\t\tRun Calendar.h, Clock.h, and Julian.h tests\n");
     fprintf(stderr, "  --dirty\tRun dirty test\n");
     fprintf(stderr, "  --display\tDisplay boundary verification results\n");
     fprintf(stderr, "  --size\tRun object size test\n");
     fprintf(stderr, "  --calendar\tRun Calender.h tests\n");
     fprintf(stderr, "  --clock\tRun Clock.h tests\n");
     fprintf(stderr, "  --julian\tRun Julian.h tests\n");
   });

   tc.on_init([](int, char**) {
     srand(time(nullptr));          // Initialize the random number generator
     return 0;
   });

   tc.on_main([tr](int, char**) {
     int error_count= 0;            // Error count

     if( opt_hcdm || opt_verbose ) {
       // Startup message
       char buffer[64];
       time_t now= time(nullptr);   // The current time
       size_t L= strftime(buffer, sizeof(buffer), "%b %d %Y %H:%M:%S"
                         , localtime(&now));
       if( L == 0 )                 // If strftime failure
         strcpy(buffer, "Today");   // Set generic date

       debugh("==============================================\n");
       debugh("======== Starting TestTime\n");
       debugh("======== Compiled %s %s\n", __DATE__, __TIME__);
       debugh("========  Started %s\n", buffer);
       debugh("==============================================\n");
       debugf("%5d= opt_hcdm\n", opt_hcdm);
       debugf("%5d= opt_verbose\n", opt_verbose);
       debugf("%5d= opt_display\n", opt_display);
     }

     try {
       if( opt_size  )    error_count += test_size();
       if( opt_clock )    error_count += test_clock();
       if( opt_julian )   error_count += test_julian();
       if( opt_calendar ) error_count += test_calendar();
       if( opt_dirty )    error_count += test_dirty();

       // Statistics
       if( opt_verbose ) {
         Reporter::get()->report([](Reporter::Record& record) {
           debugf("%s\n", record.h_report().c_str());
         }); // reporter.report
       }
     } catch(std::exception& x) {
       debugf("FAILED: Exception: exception(%s)\n", x.what());
       ++error_count;
     } catch(...) {
       debugf("FAILED: Exception: ...\n");
       ++error_count;
     }

     if( opt_verbose || error_count ) {
       debugf("\n");
       tr->report_errors(error_count);
     }

     return int(error_count != 0);
   });

   tc.on_parm([](string name, const char* value) {
     if( opt_hcdm )
       debugf("on_parm(%s,%s)\n", s2c(name), value);

     if( name == "all" ) {
       opt_calendar= true;
       opt_clock= true;
       opt_julian= true;
     }

     return 0;
   });

   //-----------------------------------------------------------------------
   // Run the tests
   opt_hcdm= HCDM;
   opt_verbose= VERBOSE;
   return tc.run(argc, argv);
}
