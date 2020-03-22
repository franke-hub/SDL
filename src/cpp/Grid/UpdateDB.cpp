//----------------------------------------------------------------------------
//
//       Copyright (c) 2011-2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       UpdateDB.cpp
//
// Purpose-
//       Update a database, changing UUIDs.
//
// Last change date-
//       2014/01/01
//
// Parameters-
//       stdin: The database to change
//       Name of change list file.
//
// Input-
//       Change list:
//       xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
//
// Output-
//       stdout: The updated database.
//
//----------------------------------------------------------------------------
#include <stdarg.h>                 // For error()
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp

#include <com/Debug.h>
#include <com/List.h>
#include <com/Reader.h>
#include <com/Writer.h>

//----------------------------------------------------------------------------
// Constant for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Class-
//       ChangeLink
//
// Purpose-
//       Describe a changeList elemment.
//
//----------------------------------------------------------------------------
class ChangeLink : public List<ChangeLink>::Link { // ChangeLink element
public:
   char                from[40];    // Change from
   char                into[40];    // Change into

inline
   ~ChangeLink( void ) {}           // Destructor

inline
   ChangeLink(                      // Constructor
     const char*       from,        // From element
     const char*       into)        // Into element
{
   strcpy(this->from, from);
   strcpy(this->into, into);
}
}; // class ChangeLink

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static char            inpBuff[65536]; // The input line buffer
static List<ChangeLink>changeList;  // List of ChangeLink elements
static const char*     CHANGE_FILE; // The change file name

//----------------------------------------------------------------------------
//
// Subroutine-
//       error
//
// Purpose-
//       Write a message onto stderr
//
//----------------------------------------------------------------------------
static void
   error(                           // Write error message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stderr, fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       readChangeList
//
// Purpose-
//       Read the change list.
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   readChangeList( void )           // Read the change list
{
   FileReader          reader(CHANGE_FILE); // The input file

   if( reader.getState() != reader.STATE_INPUT )
   {
     error("File(%s): NOT READABLE\n", CHANGE_FILE);
     exit(EXIT_FAILURE);
   }

   for(;;)
   {
     char inpLine[4096];

     int rc= reader.readLine(inpLine, sizeof(inpLine));
     if( rc == Reader::RC_EOF )
       break;

     if( rc < 0 )
     {
       error("%d= reader(%s).readLine()\n", rc, CHANGE_FILE);
       return 1;
     }

     strcpy(inpBuff, inpLine);
     char* C= inpLine;
     while( *C == ' ' )
       C++;

     if( *C == '\0' )
       continue;

     char* from= C;
     while( *C != ' ' && *C != '\0' )
       C++;

     if( *C == '\0' )
     {
       error("Malformed input: '%s'\n", inpBuff);
       return 1;
     }

     *C= '\0';
     C++;
     while( *C == ' ' )
       C++;

     if( *C == '\0' )
     {
       error("Malformed input: '%s'\n", inpBuff);
       return 1;
     }

     char* into= C;
     while( *C != ' ' && *C != '\0' )
       C++;

     if( *C == ' ' )
     {
       *C= '\0';
       C++;
       while( *C == ' ' )
         C++;

       if( *C != '\0' )
       {
         error("Malformed input: '%s'\n", inpBuff);
         return 1;
       }
     }

     if( strlen(from) != 36 )
     {
       error("Malformed from(%s) length(%d) != 36\n", from, strlen(from));
       return 1;
     }

     if( strlen(into) != 36 )
     {
       error("Malformed into(%s) length(%d) != 36\n", into, strlen(into));
       return 1;
     }

     changeList.fifo(new ChangeLink(from,into));
   }

   reader.close();

   // Verify the change list
   ChangeLink* link= changeList.getHead();
   if( link == NULL )
   {
     error("File(%s) EMPTY\n", CHANGE_FILE);
     return 1;
   }

   // Display the change list
   if( TRUE )
   {
     error("Change List:\n");
     while( link != NULL )
     {
       error("%s => %s\n", link->from, link->into);
       link= link->getNext();
     }
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       updater
//
// Purpose-
//       Update the database.
//
//----------------------------------------------------------------------------
static void
   updater( void )                  // Update the database
{
   const int           buffSize= 0x01000000; // The working buffer size
   char*               buffer0;     // Work buffer[0]

   buffer0= (char*)malloc(buffSize);
   if( buffer0 == NULL )
   {
     error("Storage shortage\n");
     exit(EXIT_FAILURE);
   }

   FileReader          reader(NULL); // STDIN
   FileWriter          writer(NULL); // STDOUT

   if( reader.getState() != reader.STATE_INPUT )
   {
     error("File(%s): NOT READABLE\n", "STDIN");
     return;
   }

   // Read and update
   char* buffer= buffer0;
   unsigned length= reader.read(buffer, buffSize);
   if( length == 0 )
   {
     error("File(%s): EMPTY\n", "STDIN");
     return;
   }

   while( length >= 36 )            // For each buffer
   {
//char remove[256];
//memcpy(remove, buffer, length);
//buffer[length]= '\0';
//debugf("%4d HCDM %4d '%s'\n", __LINE__, length, remove);

     char*    origin;               // Working buffer pointer
     unsigned remain;               // Working remaining length

     ChangeLink* link= changeList.getHead();
     while( link != NULL )
     {
       origin= buffer;
       remain= length;
       while( remain >= 36 )
       {
         char* C= (char*)memchr(origin, link->from[0], remain);
         if( C == NULL )
           remain= 0;
         else
         {
           if( memcmp(C, link->from, 36) == 0 )
             memcpy(C, link->into, 36);

           C++;
           unsigned offset= (C - origin);
           remain -= offset;
           origin = C;
         }
       }

       link= link->getNext();
     }

     writer.write(buffer, length-35);

     char temp[36];
     origin= buffer + length - 35;
     memcpy(temp, origin, 35);
     memcpy(buffer, temp, 35);

     remain= reader.read(buffer+35, buffSize-35);
     length= remain + 35;
   }

   if( length > 0 )
     writer.write(buffer, length);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter informational display.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter analysis
{
   fprintf(stderr,
           "UpdateDB: Update a database\n"
           "\n"
           "Parameters:\n"
           "  [1] The name of the change control list file\n"
           "  Input: stdin: Database to be changed\n"
           "  Output: stdout: Updated database\n"
          );

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   char*               argp;        // Argument pointer
   int                 argi;        // Argument index

   int                 ERROR;       // Error encountered indicator
   int                 HELPI;       // Help encountered indicator
   int                 verify;      // Verification control

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   ERROR= FALSE;                    // Set defaults
   HELPI= FALSE;
   verify= FALSE;
   CHANGE_FILE= NULL;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If this parameter is in switch format
     {
       if( strcmp("-help", argp) == 0 // If help request
           || strcmp("--help", argp) == 0 )
         HELPI= TRUE;

       else if( strcmp("-verify", argp) == 0 )
         verify= TRUE;

       else                         // If invalid switch
       {
         ERROR= TRUE;
         error("Invalid parameter '%s'\n", argv[argi]);
       }
     }
     else                           // If filename parameter
     {
       if( CHANGE_FILE != NULL)
       {
         ERROR= TRUE;
         error("Unexpected file name '%s'\n", argv[argi]);
       }
       else
         CHANGE_FILE= argp;
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( CHANGE_FILE == NULL )
   {
     ERROR= TRUE;
     error("Missing change list filename\n");
   }

   if( HELPI || ERROR )
   {
     if( ERROR )
       error("\n");

     info();
   }

   if( verify )
   {
     fprintf(stderr, "Source: '%s'\n", CHANGE_FILE);
   }
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
   parm(argc, argv);                // Parameter analysis

   if( readChangeList() != 0 )
     exit(EXIT_FAILURE);

   updater();                       // Update the database

   return EXIT_SUCCESS;
}

