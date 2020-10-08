//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       PowerChute.cpp
//
// Purpose-
//       Analyze PowerChute log.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define LOG_FILE "eventlog.dat"

#ifdef _OS_WIN
#define LOG_PATH "C:/Program Files (x86)/APC/PowerChute Personal Edition/"
#else
#define LOG_PATH "/C/Program Files (x86)/APC/PowerChute Personal Edition/"
#endif

#define LOG_NAME LOG_PATH LOG_FILE

// Event codes from
//     http://forums.apc.com/spaces/7
//          /ups-management-devices-powerchute-software
//          /forums/general/11727
//          /powerchute-personal-edition-3-0-2-windows-event-id-list
// fine EVENT_xxxxxxxx  0x00000091  // 00145 Lost network connection
#define EVENT_OVERVOLT  0x000000AC  // 00172 Overvoltage detected
#define EVENT_UNDERVOLT 0x000000AD  // 00173 Undervoltage detected
#define EVENT_BLACKOUT  0x000000AE  // 00174 Blackout detected
// fine EVENT_xxxxxxxx  0x000000AF  // 00175 (Alternate blackout)
#define EVENT_SHUTDOWN  0x000000B0  // 00176 (PC sent SHUTDOWN signal)
#define EVENT_HIBERNATE 0x000000B1  // 00177 (PC sent HIBERNATE signal)
// fine EVENT_APCTRIM   0x000000C8  // 00203 (APC handling overvoltage)
// fine EVENT_APCBOOST  0x000000C9  // 00204 (APC handling undervoltage)
#define EVENT_TESTFAIL  0x0000F00B  // 61451 Self test failed
#define EVENT_TESTDONE  0x0000F00C  // 61452 Self test OK
#define EVENT_NOISE     0x0000F00D  // 61453 Line noise detected
// fine EVENT_BATTERY   0x0000F00E  // 61454 Switched to battery power
#define EVENT_RESTORED  0x0000F00F  // 61455 Utility power restored
#define EVENT_LOSTCOMM  0x0000F010  // 61456 NOT COMMUNICATING
// fine EVENT_xxxxxxxx  0x0000F012  // 61458 NOT COMMUNICATING - on battery
// fine EVENT_xxxxxxxx  0x0000F013  // 61459 Battery connected
// fine EVENT_xxxxxxxx  0x0000F014  // 61460 Battery disconnected
// fine EVENT_xxxxxxxx  0x0000F015  // 61461 OVERLOADED
// fine EVENT_xxxxxxxx  0x0000F016  // 61462 NO LONGER OVERLOADED
// fine EVENT_xxxxxxxx  0x0000F017  // 61463 FAILED self-test, replace battery
// fine EVENT_xxxxxxxx  0x0000F018  // 61464 Internal fault detected
#define EVENT_RESTCOMM  0x0000F019  // 61465 Communication restored
#define EVENT_LOSTBCOM  0x0000F02B  // 61483 NOT COMMUNICATING - on battery
#define EVENT_RESTBCOM  0x0000F03D  // 61501 NOW COMMUNICATING - on battery
// fine EVENT_xxxxxxxx  0x0000F03F  // 61503 COMMUNICATIONS FAULT
#define EVENT_GREENON   0x0000F04A  // 61514 (Green mode ON)
#define EVENT_GREENOFF  0x0000F04B  // 61515 (Green mode OFF)

//----------------------------------------------------------------------------
//
// Struct-
//       Record
//
// Purpose-
//       Describe a PowerChute record.
//
//----------------------------------------------------------------------------
struct Record {
   short               year;        // Year
   short               month;       // Month  (1..12)
   short               x1;          // Unknown
   short               day;         // Day    (1..31)
   short               hour;        // Hour   (0..23)
   short               minute;      // Minute (0..59)
   short               second;      // Second (0..59)
   short               milli;       // Milli  (0..999)
   short               x2;          // Unknown
   short               x3;          // Unknown
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             length;      // The size of the record array
static Record*         buffer;      // The record array

//----------------------------------------------------------------------------
//
// Subroutine-
//       getX2
//
// Purpose-
//       Determine the type code
//
//----------------------------------------------------------------------------
static const char*                  // Return type
   getX2(                           // Decode the type code
     int               x2)          // The type code
{
   const char*         rm= "";      // Return message

   x2 &= 0x0000ffff;                // Remove sign extension
   switch( x2 )
   {
     case EVENT_BLACKOUT:
       rm= "Blackout";
       break;

     case EVENT_NOISE:
       rm= "Noise";
       break;

     case EVENT_OVERVOLT:
       rm= "Overvoltage";
       break;

     case EVENT_UNDERVOLT:
       rm= "Undervoltage";
       break;

     case EVENT_RESTORED:
       rm= "Restored";
       break;

     case EVENT_GREENON:
       rm= "Green ON";
       break;

     case EVENT_GREENOFF:
       rm= "Green OFF";
       break;

     case EVENT_HIBERNATE:
       rm= "PC told to hibernate";
       break;

     case EVENT_LOSTBCOM:
       rm= "Lost communication (battery)";
       break;

     case EVENT_LOSTCOMM:
       rm= "Lost communication";
       break;

     case EVENT_RESTBCOM:
       rm= "Communication restored (battery)";
       break;

     case EVENT_RESTCOMM:
       rm= "Communication restored";
       break;

     case EVENT_SHUTDOWN:
       rm= "PC told to shutdown";
       break;

     case EVENT_TESTDONE:
       rm= "Self-test OK";
       break;

     case EVENT_TESTFAIL:
       rm= "Self-test FAILED";
       break;

     default:
       break;
   }

   return rm;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       reader
//
// Purpose-
//       Read the data.
//
//----------------------------------------------------------------------------
static int                          // Return code
   reader( void )                   // Read the data
{
   FILE*               handle;      // File handle
   int                 L;

   length= 0x0100000;               // Buffer length
   buffer= (Record*)malloc(length); // Allocate the Buffer
   if( buffer == NULL )
     return 1;

   handle= fopen(LOG_NAME, "rb");
   if( handle == NULL )
   {
     fprintf(stderr, "File(%s): \n", LOG_NAME);
     perror("open error:");
     return 2;
   }

   L= fread(buffer, 1, length, handle);
   if( L == length )
     fprintf(stderr, "File(%s) too large\n", LOG_NAME);

   fclose(handle);
   length= L;

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       display
//
// Purpose-
//       Display the data.
//
//----------------------------------------------------------------------------
static int                          // Return code
   display( void )                  // Display the data
{
   int                 offset= 0;   // Current offset
   Record*             record= buffer; // Current record

   while( offset < length )
   {
     printf("%.2d/%.2d/%.4d %.2d:%.2d:%.2d.%.3d %.4x %.4x %.4x %s\n",
            record->month, record->day, record->year,
            record->hour, record->minute, record->second, record->milli,
            record->x1 & 0x0000ffff,
            record->x2 & 0x0000ffff,
            record->x3 & 0x0000ffff,
            getX2(record->x2)
            );

     record++;
     offset += sizeof(*record);
   }

   return 0;
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
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{
   int                 rc;          // Return code

   //-------------------------------------------------------------------------
   // Read the buffer
   //-------------------------------------------------------------------------
   rc= reader();
   if( rc != 0 )
     return 1;

   //-------------------------------------------------------------------------
   // Display the buffer
   //-------------------------------------------------------------------------
   rc= display();
   if( rc != 0 )
     return 1;

   return 0;
}

