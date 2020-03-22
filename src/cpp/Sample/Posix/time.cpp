//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       time.cpp
//
// Purpose-
//       Tests posix time, gmtime and localtime
//
// Last change date-
//       2007/01/01
//
// Description-
//       On windows, time_t values don't seem to display GMT.
//       If the time is in DST, one extra hour is added to its value!
//
//----------------------------------------------------------------------------
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/types.h>

#if defined(_OS_BSD)
//  #include <sys/time.h>
#endif

//----------------------------------------------------------------------------
// Macros (OS dependencies)
//----------------------------------------------------------------------------
#ifdef _OS_BSD
  #define _ftime   ftime
  #define _timeb   timeb
  #define _tzset   tzset
#endif

#if defined(_OS_BSD) && !defined(_OS_CYGWIN)
  #define _tzname  tzname
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static timeb           ftod;        // (Full) Time of day
static time_t          tod;         // Time of day

//----------------------------------------------------------------------------
//
// Subroutine-
//       _strdate
//
// Purpose-
//       Windows _strdate function.
//
//----------------------------------------------------------------------------
#ifdef _OS_BSD
void _strdate(char* tmpbuf)
{
   struct tm* tmp_tm= localtime(&tod);

   strftime(tmpbuf, 128, "%m/%d/%y", tmp_tm);
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       _strtime
//
// Purpose-
//       Windows _strtime function.
//
//----------------------------------------------------------------------------
#ifdef _OS_BSD
void _strtime(char* tmpbuf)
{
   struct tm* tmp_tm= localtime(&tod);

   strftime(tmpbuf, 128, "%H:%M", tmp_tm);
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       showTime
//
// Purpose-
//       Display the time and date.
//
//----------------------------------------------------------------------------
static void
   showTime( void )                 // Display date and time
{
   struct tm           t;           // Time information
   time_t              dot;         // Time of day

   t= *gmtime(&tod);
   dot= mktime(&t);
   printf("%.10ld= GMTime(%.10ld) %.2d/%.2d/%.4d %.2d:%.2d:%.2d UTC isdst(%d)\n"
          " daylight(%d) tzname(%s,%s) timeb(%ld,%d,%d,%d)\n"
          , (long)dot, (long)tod
          , t.tm_mon+1, t.tm_mday, t.tm_year+1900
          , t.tm_hour, t.tm_min, t.tm_sec, t.tm_isdst
          , daylight, tzname[0], tzname[1]
          , (long)ftod.time, ftod.millitm, ftod.timezone, ftod.dstflag
          );

   t= *localtime(&tod);
   dot= mktime(&t);
   printf("%.10ld= LCTime(%.10ld) %.2d/%.2d/%.4d %.2d:%.2d:%.2d LCL isdst(%d)\n"
          " daylight(%d) tzname(%s,%s) timeb(%ld,%d,%d,%d)\n"
          , (long)dot, (long)tod
          , t.tm_mon+1, t.tm_mday, t.tm_year+1900
          , t.tm_hour, t.tm_min, t.tm_sec, t.tm_isdst
          , daylight, tzname[0], tzname[1]
          , (long)ftod.time, ftod.millitm, ftod.timezone, ftod.dstflag
          );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Function-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   ftime(&ftod);
   time(&tod);
   if( argc > 1 )
     tod= atol(argv[1]);

   showTime();
   return 0;
}

