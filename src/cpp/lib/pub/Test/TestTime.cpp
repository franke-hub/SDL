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
//       2025/04/26
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
//       verify_day
//
// Purpose-
//       Verify Julian date and calendar/clock consistency
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE if values are (approximately) equal
   verify_day(                      // Verify                                     // Cut/paste source
     double            date,        // This Julian date
     int               verbosity= opt_verbose)
{
   bool                OK= true;    // (So far, so good)

   Julian rhj(date);
   Calendar calendar(rhj);
   Julian lhj= (Julian)calendar;

   double PRECISION= 1000000.0;     // Require microsecond precision
   int64_t lhi= int64_t( (double)lhj * PRECISION );
   int64_t rhi= int64_t( (double)rhj * PRECISION );
   if( lhi != rhi ) {
     debugf("%4d verify_day NG: %'14.6f = %s = %'14.6f\n", __LINE__
           , lhi / PRECISION, s2c((string)calendar), rhi / PRECISION);
     OK= false;
   }

   Clock rhc= (Clock)rhj;
   Julian julian(rhc);
   Clock lhc= (Clock)julian;

   lhi= int64_t( (double)lhc * PRECISION );
   rhi= int64_t( (double)rhc * PRECISION );
   if( abs(lhi - rhi) > 1 ) {
     debugf("%4d verify_day NG: %s\n"
            "     Clock(%'.3f) = Julian(%'.6f) = Clock(%'.3f)\n", __LINE__
           , s2c((string)calendar)
           , lhi / PRECISION, julian.get(), rhi / PRECISION);
     OK= false;
   }
   if( OK && (verbosity > 1) ) {
     debugf("verify_day OK: %02d/%02d/%04zd%s  %'18.8f  %'20.3f\n"
           , calendar.get_month(), calendar.get_day(), calendar.get_year()
           , calendar.get_year() >= -999 ? " " : ""
           , lhj.get(), rhc.get());
   }

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

   // Verify the Calendar range -5000 .. 5000
   double MIN_JULIAN= -105192;      // 1/1/-5000 Julian
   double MAX_JULIAN= 3547273;      // 1/1/+5000 Gregorian
   int verbosity= opt_verbose;
   if( verbosity > 1 || opt_display ) {
     debugf("\nVerifying Calendar day range:\n");
     debugf("               --- year --   -- Julian date --  ---- Clock time ----\n");
   }

   for(int64_t day= MIN_JULIAN; day <MAX_JULIAN; ++day) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( day < (MIN_JULIAN + 15) ) v_verbosity= 2;
       if( day > (MAX_JULIAN - 15) ) v_verbosity= 2;
     }

     error_count += VERIFY( verify_day(day, v_verbosity) );
     if( error_count > 16 )         // (Broken code escape)
       break;
   }

   // Verify the time of day 0.0 .. 1.0
   if( verbosity > 1 || opt_display ) {
     debugf("\nVerifying Calendar time of day range:\n");
     debugf("               --- year --   -- Julian date --  ---- Clock time ----\n");
   }

   double increment= 0.123456789 / 864000.0;
   for(double tod= increment; tod < 1.0; tod += increment) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( tod <= 0.0 + increment * 15 ) v_verbosity= 2;
       if( tod >= 1.0 - increment * 15 ) v_verbosity= 2;
     }

     error_count += VERIFY( verify_day(MIN_JULIAN - tod, v_verbosity) );
     if( error_count > 16 )         // (Broken code escape)
       break;
   }

   for(double tod= 0.0; tod < 1.0; tod += increment) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( tod <= 0.0 + increment * 15 ) v_verbosity= 2;
       if( tod >= 1.0 - increment * 15 ) v_verbosity= 2;
     }

     error_count += VERIFY( verify_day(MIN_JULIAN + tod, v_verbosity) );
     if( error_count > 16 )         // (Broken code escape)
       break;
   }

   if( verbosity > 1 || opt_display ) {
     debugf("\n");
     debugf("               --- year --   -- Julian date --  ---- Clock time ----\n");
   }
   for(double tod= increment; tod < 1.0; tod += increment) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( tod <= 0.0 + increment * 15 ) v_verbosity= 2;
       if( tod >= 1.0 - increment * 15 ) v_verbosity= 2;
     }

     error_count += VERIFY( verify_day(MAX_JULIAN - tod, v_verbosity) );
     if( error_count > 16 )         // (Broken code escape)
       break;
   }

   for(double tod= 0.0; tod < 1.0; tod += increment) {
     int v_verbosity= verbosity;    // Verification verbosity
     if( opt_display ) {
       if( tod <= 0.0 + increment * 15 ) v_verbosity= 2;
       if( tod >= 1.0 - increment * 15 ) v_verbosity= 2;
     }

     error_count += VERIFY( verify_day(MAX_JULIAN + tod, v_verbosity) );
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

   Clock L= Clock::now();
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

   Julian L= Julian::now();
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
//       Cut/paste sample test.
//
//----------------------------------------------------------------------------
static inline int
   test_case( void )                // Cut/paste source
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
   opt_verbose= true;
   if( opt_verbose )
     debugf("\ntest_dirty:\n");

   int error_count= 0;

   //-------------------------------------------------------------------------
   // Special dates
   if( true ) {
     opt_verbose= 1;
     Julian   julian(0.0);
     Calendar calendar(julian);

     calendar.setYMDHMS(-5000,  1,  1); // Julian
     julian= (Julian)calendar;
     calendar.set(julian);
     verify_day((double)julian);

     calendar.setYMDHMS(-4000,  1,  1); // Julian
     julian= (Julian)calendar;
     calendar.set(julian);
     verify_day((double)julian);

     calendar.setYMDHMS(   -1,  1,  1); // Julian
     julian= (Julian)calendar;
     calendar.set(julian);
     verify_day((double)julian);

     calendar.setYMDHMS(    0,  1,  1); // Julian
     julian= (Julian)calendar;
     calendar.set(julian);
     verify_day((double)julian);

     calendar.setYMDHMS(    1,  1,  1); // Julian
     julian= (Julian)calendar;
     calendar.set(julian);
     verify_day((double)julian);

     calendar.setYMDHMS( 1000,  1,  1); // Julian
     julian= (Julian)calendar;
     calendar.set(julian);
     verify_day((double)julian);

     calendar.setYMDHMS( 1600,  1,  1); // Gregorian
     julian= (Julian)calendar;
     calendar.set(julian);
     verify_day((double)julian);

     calendar.setYMDHMS( 1700,  1,  1); // Gregorian
     julian= (Julian)calendar;
     calendar.set(julian);
     verify_day((double)julian);

     calendar.setYMDHMS( 1800,  2,  1); // Gregorian
     julian= (Julian)calendar;
     calendar.set(julian);
     verify_day((double)julian);

     calendar.setYMDHMS( 1900,  1,  1); // Gregorian
     julian= (Julian)calendar;
     calendar.set(julian);
     verify_day((double)julian);

     calendar.setYMDHMS( 2000,  1,  1); // Gregorian
     julian= (Julian)calendar;
     calendar.set(julian);
     verify_day((double)julian);

     calendar.setYMDHMS( 3000,  1,  1); // Gregorian
     julian= (Julian)calendar;
     calendar.set(julian);
     verify_day((double)julian);

     calendar.setYMDHMS( 4000,  1,  1); // Gregorian
     julian= (Julian)calendar;
     calendar.set(julian);
     verify_day((double)julian);

     calendar.setYMDHMS( 9999, 12, 31); // Gregorian
     julian= (Julian)calendar;
     calendar.set(julian);
     verify_day((double)julian);

     calendar.setYMDHMS(10000,  1,  1); // Gregorian
     julian= (Julian)calendar;
     calendar.set(julian);
     verify_day((double)julian);
   }

   if( false ) {
     opt_verbose= 1;
     Julian   julian;
     Calendar calendar;

     calendar.setYMDHMS(-5000,  1,  1); // Julian
     julian= (Julian)calendar;
     verify_day((double)julian);

     calendar.setYMDHMS(-4000,  1,  1); // Julian
     julian= (Julian)calendar;
     verify_day((double)julian);

     calendar.setYMDHMS(   -1,  1,  1); // Julian
     julian= (Julian)calendar;
     verify_day((double)julian);

     calendar.setYMDHMS(    0,  1,  1); // Julian
     julian= (Julian)calendar;
     verify_day((double)julian);

     calendar.setYMDHMS(    1,  1,  1); // Julian
     julian= (Julian)calendar;
     verify_day((double)julian);

     calendar.setYMDHMS( 1000,  1,  1); // Julian
     julian= (Julian)calendar;
     verify_day((double)julian);

     calendar.setYMDHMS( 1600,  1,  1); // Gregorian
     julian= (Julian)calendar;
     verify_day((double)julian);

     calendar.setYMDHMS( 2000,  1,  1); // Gregorian
     julian= (Julian)calendar;
     verify_day((double)julian);

     calendar.setYMDHMS( 3000,  1,  1); // Gregorian
     julian= (Julian)calendar;
     verify_day((double)julian);

     calendar.setYMDHMS( 4000,  1,  1); // Gregorian
     julian= (Julian)calendar;
     verify_day((double)julian);

     calendar.setYMDHMS( 9999, 12, 31); // Gregorian
     julian= (Julian)calendar;
     verify_day((double)julian);

     calendar.setYMDHMS(10000,  1,  1); // Gregorian
     julian= (Julian)calendar;
     verify_day((double)julian);
   }

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
