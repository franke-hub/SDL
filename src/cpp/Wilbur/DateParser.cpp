//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DateParser.cpp
//
// Purpose-
//       DateParser implementation methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <com/istring.h>
#include <com/Clock.h>
#include <com/Debug.h>
#include <com/Calendar.h>
#include <com/Julian.h>
#include <com/Parser.h>

#include "DateParser.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef BRINGUP
#undef  BRINGUP                     // If defined, BRINGUP Mode
#endif

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     dow[7]=      // Day of Week string array
{  "Mon"
,  "Tue"
,  "Wed"
,  "Thu"
,  "Fri"
,  "Sat"
,  "Sun"
};

static const char*     moy[12]=     // Month of Year string array
{  "Jan"
,  "Feb"
,  "Mar"
,  "Apr"
,  "May"
,  "Jun"
,  "Jul"
,  "Aug"
,  "Sep"
,  "Oct"
,  "Nov"
,  "Dec"
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       charToString
//
// Purpose-
//       Generate a std::string from a const char*
//
//----------------------------------------------------------------------------
static std::string                  // Resultant
   charToString(                    // Generate substring from C-string
     const char*       source,      // The source string
     int               length)      // The source length
{
   std::string result(source, length);
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parseDec
//
// Purpose-
//       Parse a string, extracting a decimal value.
//       Leading blanks are skipped.
//
// Returns-
//       Return (decimal value)
//       String (The value delimiter)
//
//----------------------------------------------------------------------------
static int                          // Return value
   parseDec(                        // Extract decimal value from string
     const char*&      C)           // -> String (updated)
{
   Parser parser(C);
   int result= parser.toDec();
   C= parser.getString();
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DateParser::generate
//
// Purpose-
//       Generate an RFC1123 date string.
//
//----------------------------------------------------------------------------
std::string                         // Resultant
   DateParser::generate(            // Date string generator
     time_t            source)      // The source date
{
   Clock               clock(source); // Clock from time_t
   Julian              julian(clock); // Julian from Clock
   Calendar            calendar(julian); // The associated Calendar
   char                buffer[64];  // Result buffer

   int dowX= int64_t(julian.getDate()) % 7; // Get day of week index
   int moyX= calendar.getMonth() - 1; // Get month of year index
   sprintf(buffer, "%s, %.2d %s %ld %.2d:%.2d:%.2d GMT",
           dow[dowX], calendar.getDay(), moy[moyX], (long)calendar.getYear(),
           calendar.getHour(), calendar.getMinute(), calendar.getSecond());

   std::string result(buffer);
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DateParser::parse
//
// Purpose-
//       Parse a date string.
//
//----------------------------------------------------------------------------
time_t                              // Resultant
   DateParser::parse(               // Date string parser
     const char*       source)      // The source string
{
   Calendar            calendar;    // The associated Calendar
   const char*         C;           // token.c_str()
   std::string         token;       // String token
   int                 x;           // String index

   long                year= 0;     // Resultant year
   int                 month= 1;    // Resultant month (of year)
   int                 day= 1;      // Resultant day (of month)
   int                 hour= 0;     // Resultant hour (of day)
   int                 minute= 0;   // Resultant minute (of hour)
   int                 second= 0;   // Resultant second (of minute)

   while( *source == ' ' )
     source++;

   for(x= 0; source[x] != ' ' && source[x] != '\0'; x++)
     ;;

   //-------------------------------------------------------------------------
   // Form DOW MOY dd hh:mm:ss yyyy
   if( x == 3 )                      // Form DOW MOY dd hh:mm:ss yyyy
   {
     source += 3;
     while( *source == ' ' )
       source++;

     for(x= 0; source[x] != ' ' && source[x] != '\0'; x++)
       ;;
     token= charToString(source, x);

     C= token.c_str();
     for(month= 0; month<12; month++)
     {
       if( stricmp(C, moy[month]) == 0 )
         break;
     }
     month++;

     source += x;
     day= parseDec(source);

     hour= parseDec(source);
     if( *source == ':' )
       source++;
     minute= parseDec(source);
     if( *source == ':' )
       source++;
     second= parseDec(source);

     year= parseDec(source);
   }

   //-------------------------------------------------------------------------
   // Form DOW, dd MOY yyyy hh:mm:ss GMT
   else if( x == 4 )
   {
     source += 4;
     day= parseDec(source);

     while( *source == ' ' )
       source++;

     for(x= 0; source[x] != ' ' && source[x] != '\0'; x++)
       ;;
     token= charToString(source, x);

     C= token.c_str();
     for(month= 0; month<12; month++)
     {
       if( stricmp(C, moy[month]) == 0 )
         break;
     }
     month++;

     source += x;
     year= parseDec(source);

     hour= parseDec(source);
     if( *source == ':' )
       source++;
     minute= parseDec(source);
     if( *source == ':' )
       source++;
     second= parseDec(source);
   }

   //-------------------------------------------------------------------------
   // Form DOWday, dd-MOY-yy hh:mm:ss GMT
   else if( x == 7 )
   {
     source += 7;
     while( *source == ' ' )
       source++;

     day= parseDec(source);
     if( *source == '-' )
       source++;

     for(x= 0; source[x] != '-' && source[x] != '\0'; x++)
       ;;
     token= charToString(source, x);

     C= token.c_str();
     for(month= 0; month<12; month++)
     {
       if( stricmp(C, moy[month]) == 0 )
         break;
     }
     month++;

     source += x;
     if( *source == '-' )
       source++;

     year= parseDec(source);
     if( year < 70 )
       year += 2000;
     else
       year += 1900;

     hour= parseDec(source);
     if( *source == ':' )
       source++;
     minute= parseDec(source);
     if( *source == ':' )
       source++;
     second= parseDec(source);
   }

   try {                            // Ignore range errors
     calendar.setYMDHMSN(year, month, day, hour, minute, second, 0);
   } catch(...) {
     traceh("%4d %s Calendar.set exception\n", __LINE__, __FILE__);
   }

   Clock clock= calendar.toClock();
   return long(clock.getTime());
}

