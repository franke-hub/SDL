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
//       Service.cpp
//
// Purpose-
//       Service control command.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/Thread.h>
#include <com/Unconditional.h>

#include "com/Service.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define SERVICE_PAGE_SIZE      4096 // Page size

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Information exit.
//
//----------------------------------------------------------------------------
static void
   info(                            // Information exit
     const char*       command)     // Command name
{
   fprintf(stderr, "%s {start | reset | info}\n", command);
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dumpData
//
// Function-
//       Dump data area.
//
//----------------------------------------------------------------------------
void
   dumpData(                        // Diagnostic display
     const void*       inpAddr,     // Physical origin
     unsigned          inpSize)     // Display length
{
   const char*         const addr= (const char*)inpAddr; // Input address

   enum                             // Local constants
   {
     BytesPerLine=               32,// Number of bytes per line
     WordsPerLine= (BytesPerLine/4) // Number of words per line
   }; // enum
   enum                             // States
   {
     FSM_FIRST,                     // FIRST - first file line
     FSM_UNDUP,                     // UNDUP - not duplicating
     FSM_INDUP                      // INDUP - duplicating lines
   }                   fsm;         // Finite State Machine

   long*               ptrWord;     // -> Word
   unsigned char*      ptrChar;     // -> Character

   char                tString[64]; // Text String
   char                curLine[BytesPerLine+1]; // Current line data
   char                repLine[BytesPerLine];   // Repeated line data
   char                outWord[WordsPerLine][9];// Line format area

   unsigned            offset;      // Current offset
   unsigned            origin;      // Duplication origin
   unsigned            remain;      // Remaining length
   unsigned            wc;          // Number of words on this line

   unsigned int        i;

   //-------------------------------------------------------------------------
   // Format lines
   //-------------------------------------------------------------------------
   fsm= FSM_FIRST;                  // This is the first line
   curLine[BytesPerLine]= '\0';     // String delimiter
   offset= 0;                       // Begin at the beginning
   origin= 0;                       // Not needed, but prevents error message
   remain= inpSize;
   for(;;)                          // Format lines
   {
     wc= WordsPerLine;              // Default, full line
     if( remain < BytesPerLine )
     {
       wc= (remain+3)/4;
       memset(curLine, 0, BytesPerLine);
       memcpy(curLine, (addr+offset), remain);
       for(i= wc; i<WordsPerLine; i++)
         strcpy(outWord[i], "~~~~~~~~");
     }
     else
       memcpy(curLine, (addr+offset), BytesPerLine);

     switch(fsm)                    // Process by state
     {
       case FSM_UNDUP:              // If UNDUP state
         if( remain <= BytesPerLine ) // If this is the last line
           break;                   // Do not go into INDUP state

         if( memcmp(curLine, repLine, BytesPerLine) == 0 ) // If duplicate line
         {
           fsm= FSM_INDUP;          // Go into duplicate state
           origin= offset;          // Save origin
         }
         break;

       case FSM_INDUP:              // If INDUP state
         if( remain <= BytesPerLine
             || memcmp(curLine, repLine, BytesPerLine) != 0 )
         {
           fsm= FSM_UNDUP;
           debugf("%.6X  to %.6X, lines same as above\n",
                   origin, offset-1);
         }

         break;

       default:
         fsm= FSM_UNDUP;            // Go into UNDUP state
         break;
     }

     tString[0]= '\0';
     if( offset > 4096 )            // If trace data
     {
       if( Service::word(curLine) == Service::word(".BUG") )
       {
         Service::DebugRecord* record= (Service::DebugRecord*)curLine;
         sprintf(tString, "Line(%4d) Data(0x%.8X)",
                          record->line, record->data);
       }
     }

     if( fsm == FSM_UNDUP )
     {
       memcpy(repLine, curLine, BytesPerLine); // Save the line for compare
       ptrWord= (long*)(curLine);
       for(i=0; i<wc; i++)
       {
         ptrChar= (unsigned char*)(ptrWord+i);
         sprintf(outWord[i], "%.2X%.2X%.2X%.2X",
                             ptrChar[0], ptrChar[1],
                             ptrChar[2], ptrChar[3]);
       }

       for(i=0; i<BytesPerLine; i++)
       {
         if( !isprint(curLine[i]) )
           curLine[i]= '.';
       }

       for(i=remain; i<BytesPerLine; i++)
         curLine[i]= '~';

       debugf("%.6X: %s %s %s %s  %s %s %s %s  |%.32s| %s\n", offset,
              outWord[0], outWord[1], outWord[2], outWord[3],
              outWord[4], outWord[5], outWord[6], outWord[7],
              curLine, tString);
     }

     if( remain <= BytesPerLine )
       break;

     offset += BytesPerLine;
     remain -= BytesPerLine;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dump
//
// Purpose-
//       Process "info" function.
//
//----------------------------------------------------------------------------
static inline void
   dump( void )                     // Handle info function
{
   unsigned            length= Service::getLength();
   size_t              offset;

   char*               result;      // Storage area
   Service::Global*    global;      // Local copy

   SERVICE_INFO(-1);
   if( length == 0 )
   {
     fprintf(stderr, "Not active\n");
     return;
   }

   result= (char*)must_malloc(length + SERVICE_PAGE_SIZE);
   memset(result, 0, length + SERVICE_PAGE_SIZE);

   offset= (size_t)result;
   offset &= (SERVICE_PAGE_SIZE-1);
   offset= SERVICE_PAGE_SIZE - offset;
   global= (Service::Global*)(result + offset);
   Service::info(global);
   dumpData(global, length);
   free(result);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       start
//
// Purpose-
//       Process "start" function.
//
//----------------------------------------------------------------------------
static inline void
   start( void )                    // Handle start function
{
   Service::start();

   #if defined(_OS_WIN) || defined(_OS_CYGWIN)
     printf("Service started\n");
     for(;;)
       Thread::sleep(600.0);
   #endif
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
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Process Service command
   //-------------------------------------------------------------------------
   if( argc != 2 )
     info(argv[0]);

   if( strcmp(argv[1], "info") == 0 )
     dump();

   else if( strcmp(argv[1], "start") == 0 )
     start();

   else if( strcmp(argv[1], "reset") == 0 )
     Service::reset();

   else
     info(argv[0]);

   return 0;
}

