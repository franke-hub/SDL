//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       UtilArchive.h
//
// Purpose-
//       Define and implement the Archive utility subroutines.
//
// Last change date-
//       2014/01/01
//
// Implementation notes-
//       Included from Archive.cpp
//       Includes utility subroutines.
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugSTREAM
//
// Purpose-
//       Display STREAM information.
//
//----------------------------------------------------------------------------
static inline void
   debugSTREAM(                     // Display STREAM information
     z_stream&         stream)      // For this zstream
{
   debugf("debugSTREAM(%p)\n", &stream);
   debugf(".  next_in(%p)\n",   stream.next_in);
   debugf(". avail_in(%8d)\n",  stream.avail_in);
   debugf(". total_in(%8ld)\n", stream.total_in);
   debugf(". next_out(%p)\n",   stream.next_out);
   debugf(".avail_out(%8d)\n",  stream.avail_out);
   debugf(".total_out(%8ld)\n", stream.total_out);
   debugf(".      msg(%s)\n",   stream.msg);
   debugf(".   opaque(%p)\n",   stream.opaque);
   debugf(".data_type(%8d)\n",  stream.data_type);
   debugf(".    adler(%8lx)\n", stream.adler);
   debugf(". reserved(%8ld)\n", stream.reserved);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       FAT_DATE_TIME
//
// Purpose-
//       Convert FAT date/time to time_t
//
// Format-
//       DATE: yyyy yyym mmmd dddd Year, Month, Day (Add 1980 to year)
//       TIME: hhhh hmmm mmms ssss Hour, Minute, Second/2
//
//----------------------------------------------------------------------------
static time_t                       // Resultant
   FAT_DATE_TIME(                   // Convert FAT date/time
     int               date,        // (16 bit) date
     int               time)        // (16 bit) time
{
   int year= (date & 0x0000FE00) >>  9;
       year+= 1980;
   int  moy= (date & 0x000001E0) >>  5;
   int  dom= (date & 0x0000001F);

   int hour= (time & 0x0000F800) >> 11;
   int  min= (time & 0x000007E0) >>  5;
   int  sec= (time & 0x0000001F);
        sec*= 2;

   // Ignoring the time zone, convert the date/time into a time_t
   Calendar calendar;

   calendar.setYMDHMSN(year, moy, dom, hour, min, sec);
   Clock clock= calendar.toClock();
   time_t result= (time_t)clock.getTime();

   return result;
}

