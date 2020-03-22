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
//       FileIO.cpp
//
// Purpose-
//       Sample file I/O usage.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <com/Debug.h>
#include <com/define.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#define SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#define IODM                        // If defined, Input/Output Debug Mode
#endif

#ifndef USE_FILEIO
#define USE_FILEIO                  // If defined, use FILE* I/O
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define NAME "erase.me"

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             isReader;    // This is the reader
static int             swDebug;     // Debugging control

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Informational exit
//
//----------------------------------------------------------------------------
static void
   info(                            // Informational exit
     const char*     sourceName)    // The source fileName
{
   fprintf(stderr, "%s <options> {reader || writer}\n", sourceName);
   fprintf(stderr, "\n");
   fprintf(stderr, "Options:\n");
   fprintf(stderr, "-v\tVerify parameters\n");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Analyze parameters
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   int               error;         // TRUE if error encountered
   int               verify;        // TRUE if verify required

   int               i, j;

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no error found
   verify= FALSE;

   isReader= TRUE;

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   for(j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp(argv[j], "-help") == 0 )
         error= TRUE;

       else
       {
         for(i=1; argv[j][i] != '\0'; i++) // Examine the switch list
         {
           switch(argv[j][i])       // Examine the switch
           {
             case 'd':              // -d (debug)
               swDebug= TRUE;
               break;

             case 'h':              // -h (help)
               error= TRUE;
               break;

             case 'v':              // -v (verify)
               verify= TRUE;
               break;

             default:               // If invalid switch
               error= TRUE;
               fprintf(stderr, "Invalid switch '%c'\n", (int)argv[j][i]);
               break;
           }
         }
       }
     }
     else if( strcmp(argv[j], "reader") == 0 )
     {
       isReader= TRUE;
     }
     else if( strcmp(argv[j], "writer") == 0 )
     {
       isReader= FALSE;
     }
     else                           // Argument
     {
       error= TRUE;
       fprintf(stderr, "Invalid parameter: '%s'\n", argv[j]);
     }
   }

   //-------------------------------------------------------------------------
   // Check and validate
   //-------------------------------------------------------------------------
   if( error )
     info(argv[0]);

   if( verify )
   {
     fprintf(stderr, "%10d reader\n", isReader);
     fprintf(stderr, "%10d debug\n", swDebug);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       reader
//
// Purpose-
//       Run the reader.
//
//----------------------------------------------------------------------------
#ifdef USE_FILEIO
static void
   reader( void )                   // Run the reader
{
   FILE*               handle;      // The file handle
   char                string[128]; // Working string

   int                 L;

   handle= fopen(NAME, "r");
   #ifdef IODM
     printf("%p= fopen(%s,%s)\n", handle, NAME, "r");
   #endif
   if( handle == NULL )
   {
     perror("Open failed");
     return;
   }

   for(;;)
   {
     L= fread(string, 1, sizeof(string), handle);
     #ifdef IODM
       printf("%d= fread(%p,%d,%zd,%p)\n", L, string, 1, sizeof(string), handle);
     #endif

     if( L < 0 )
       break;

     if( L ==  0 )
       usleep(100000);
     else
       printf("%s\n", string);
   }
}

#else
static void
   reader( void )                   // Run the reader
{
   int                 handle;      // The file handle
   char                string[128]; // Working string

   int                 L;

   handle= open(NAME, 0);
   #ifdef IODM
     printf("%d= open(%s,%d)\n", handle, NAME, 0);
   #endif
   if( handle < 0 )
   {
     perror("Open failed");
     return;
   }

   for(;;)
   {
     L= read(handle, string, sizeof(string));
     #ifdef IODM
       printf("%d= read(%p,%d)\n", L, string, sizeof(string));
     #endif

     if( L < 0 )
       break;

     if( L ==  0 )
       usleep(100000);
     else
       printf("%s\n", string);
   }
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       writer
//
// Purpose-
//       Run the writer.
//
//----------------------------------------------------------------------------
static void
   writer( void )                   // Run the reader
{
   int                 handle;      // The file handle
   char                string[128]; // Working string
   mode_t              mode;        // File mode

   int                 L;

   mode= S_IRUSR | S_IWUSR;
   handle= creat(NAME, mode);
   #ifdef IODM
     printf("%d= creat(%s,%x)\n", handle, NAME, mode);
   #endif
   if( handle < 0 )
   {
     perror("Open failed");
     return;
   }


   for(int item= 1; item<100; item++)
   {
     memset(string, 0, sizeof(string));
     sprintf(string, "Item %.4d", item);

     L= write(handle, string, sizeof(string));
     #ifdef IODM
       printf("%d= write(%s,%zd)\n", L, string, sizeof(string));
     #endif

     if( L != sizeof(string) )
       break;

     // sleep(1);
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
int                                 // Return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   //-------------------------------------------------------------------------
   // Parameter validation
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Process the function
   //-------------------------------------------------------------------------
   if( isReader )
     reader();
   else
     writer();

   //-------------------------------------------------------------------------
   // Return to caller
   //-------------------------------------------------------------------------
   return 0;
}

