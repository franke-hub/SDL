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
//       Test_Cal.cpp
//
// Purpose-
//       Test Calendar functions.
//
// Last change date-
//       2007/01/01
//
// Notes-
//       Input: Calendar day: mm dd year
//       Output: Julian day
//
//----------------------------------------------------------------------------
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <com/Debug.h>
#include <com/Exception.h>
#include <com/Random.h>

#include "com/Clock.h"
#include "com/Calendar.h"
#include "com/Julian.h"

//----------------------------------------------------------------------------
//
// Subroutine-
//       printCalendar
//
// Purpose-
//       Display a Calendar object
//
//----------------------------------------------------------------------------
static void
   printCalendar(                   // Display a Calendar object
     const Calendar&   c)           // The Calendar object
{
   printf("%.2d/%.2d/%.4" PRId64 ",%.2d:%.2d:%.2d.%.3d\n",
          c.getMonth(), c.getDay(), c.getYear(),
          c.getHour(), c.getMinute(), c.getSecond(), c.getMillisecond());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verifyJulian
//
// Purpose-
//       Verify the Julian object
//
//----------------------------------------------------------------------------
static void
   verifyJulian( void )             // Verify the Julian object
{
   Julian L, R;

   L= Julian::current();
   R= L;
   if( R < L || L > R )
     throw "ShouldNotOccur 1";

   L= 101;
   L -= 1;
   R= 100;
   if( L != R )
     throw "ShouldNotOccur 2";

   L= 101;
   R= 100;
   R += 1;
   if( L != R )
     throw "ShouldNotOccur 3";

   Julian result= L + R;
   if( result.getTime() != 202 )
     throw "ShouldNotOccur 4.1";

   result= L - R;
   if( result.getTime() != 0 )
     throw "ShouldNotOccur 5";

   L= 100.000000001; R= 0.000000002; L -= R;
   if( int(L.getTime()) != 99 )
     throw "ShouldNotOccur 6";

   result= L - R;
   if( int(result.getTime()) != 99 )
     throw "ShouldNotOccur 7";

   L= (-10.5 * Julian::SECONDS_PER_DAY);
   if( int(L.getDate()) != (-10) )
   {
     printf("%d\n", int(L.getDate()));
     throw "ShouldNotOccur 9.1";
   }

   L= (+10.25 * Julian::SECONDS_PER_DAY);
   if( int(L.getDate()) != (+10) )
     throw "ShouldNotOccur 9.2";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       builtinTest
//
// Purpose-
//       Verify the Julian object
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   builtinTest( void )              // Verify the Julian object
{
   int                 result= 0;   // Test return code

   Calendar            c1;          // Working Calendar
   Random              random;      // Random number
   random.randomize();

   printf("\n\n");
   printf("Running built-in tests\n");
   try {
     verifyJulian();                // Verify the object

     int yy= 1582;                  // The year
     for(unsigned moy= 9; moy<13; moy++) // The month
     {
       for(unsigned dom= 1; dom<31; dom++) // The day
       {
         if( moy == 10 && dom == 5 ) // If switching day
           dom= 15;

         unsigned hh= random.modulus(24);
         unsigned mm= random.modulus(60);
         unsigned ss= random.modulus(60);
         unsigned ms= random.modulus(1000);

         #if FALSE
           printf("cal.set(%d/%d/%d,%d:%d:%d.%.3d\n",
                  yy, moy, dom, hh, mm, ss, ms);
         #endif

         c1.setYMDHMSN(yy, moy, dom, hh, mm, ss, ms);
         Julian j= c1.toJulian();
         Calendar c2(j);
         if( c1.compare(c2) != 0 )
         {
           printf("%.9f Date\n", j.getDate());
           printf("C1: "); printCalendar(c1);
           printf("C2: "); printCalendar(c2);
           throw "Calendar crosscheck failure";
         }

         printf("%.9f = ", j.getDate()); printCalendar(c1);
       }
     }

     printf("\n");
     Julian j;
     c1= j;
     printf("%.9f, %.9f = ", j.getTime(), j.getDate()); printCalendar(c1);
   } catch(char * X) {
     fprintf(stderr, "Exception(%s)\n", X);
     result= 1;
   } catch(const char * X) {
     fprintf(stderr, "Exception(const(%s))\n", X);
     result= 1;
   } catch(Exception& X) {
     fprintf(stderr, "Exception(%s)\n", X.what());
     result= 1;
   } catch(...) {
     fprintf(stderr, "Exception(...)\n");
     result= 1;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       usage
//
// Purpose-
//       Usage information, and exit.
//
//----------------------------------------------------------------------------
static void
   usage( void )                    // Usage information
{
   fprintf(stderr, "Usage: mm dd year\n"
                   "Input the calendar month, day, and year\n"
                   "Output is the Julian day\n");
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
extern int
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   Calendar            calendar;    // Calendar
   Calendar            crosscheck;  // Crosscheck Calendar
   Julian              julian;      // Julian Date/Time

   int                 resultant= 1;// Return code
   int                 mm;          // Month
   int                 dd;          // Day
   int                 yy;          // Year

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   if( argc != 4 )
   {
     usage();
     int rc = builtinTest();
     exit(rc);
   }

   mm= dd= yy= (-1);
   mm= atoi(argv[1]);
   dd= atoi(argv[2]);
   yy= atoi(argv[3]);
   if( mm <= 0 || dd <= 0 )
   {
     usage();
     exit(1);
   }

   //-------------------------------------------------------------------------
   // Convert to Julian day
   //-------------------------------------------------------------------------
   try {
     calendar.setYMD(yy, mm, dd);
     julian= calendar.toJulian();
     crosscheck.set(julian);
     printf("%.4f %d %d %" PRId64 "\n", julian.getDate(),
            crosscheck.getMonth(), crosscheck.getDay(),
            crosscheck.getYear());

     if( calendar != crosscheck
         || crosscheck.getYear() != yy
         || crosscheck.getMonth() != mm
         || crosscheck.getDay() != dd )
     {
       printf("CA: "); printCalendar(calendar);
       printf("CC: "); printCalendar(crosscheck);
       throw "Test_Cal.CrossCheckException";
     }

     if( calendar.getYear() >= 1970 && calendar.getYear() < 2038 )
     {
       Clock c1= calendar.toClock();
       Julian j(c1);
       Clock c2(j);
       if( c1 != c2 )
       {
         throw "Test_Cal.JulianToClockException";
       }
     }

     resultant= 0;
   } catch(char * X) {
     fprintf(stderr, "Exception(%s)\n", X);
   } catch(const char * X) {
     fprintf(stderr, "Exception(const(%s))\n", X);
   } catch(Exception& X) {
     fprintf(stderr, "Exception(%s)\n", X.what());
   } catch(...) {
     fprintf(stderr, "Exception(...)\n");
   }

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   return resultant;
}

