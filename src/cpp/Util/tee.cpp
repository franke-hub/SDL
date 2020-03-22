//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       tee.cpp
//
// Purpose-
//       Copy stdin to stdout and to specified file.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/define.h>
#include <com/params.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define BUFF_SIZE              8192 // Buffer size

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static char*              fileName; // The output filename
static char               buff[BUFF_SIZE]; // The input/output buffer

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter fault exit.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter fault exit
{
   fprintf(stderr, "tee filename\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Copy stdin to stdout and to specified file\n");
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
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   char*             argp;          // Argument pointer
   int               argi;          // Argument index

   int               error;         // Error encountered indicator
// int               verify;        // Verification control

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   fileName= NULL;                  // No files found yet
   error= FALSE;                    // Default, no errors found
// verify= 0;                       // Default, no verification

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If this parameter is in switch format
     {
       argp++;                      // Skip over the switch char

//     if( swname("verify", argp) ) // If verify switch
//       verify= swatob("verify", argp); // Get switch value
//
//     else                         // If invalid switch
       {
         error= TRUE;
         fprintf(stderr, "Invalid parameter '%s'\n",
                         argv[argi]);
       }
     }
     else                           // If filename parameter
     {
       if( fileName != NULL )
       {
         error= TRUE;
         fprintf(stderr, "Too many filenames(%s)\n", argp);
       }

       fileName= argp;
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( fileName == NULL )           // If no filename specified
   {
     error= TRUE;
     fprintf(stderr, "Filename missing\n");
   }

   if( error )                      // If error encountered
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
int                                 // Main return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   FILE*             file;          // FILE handle
   int               size;          // Buffer length

   int               L;

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Open the duplicate file
   //-------------------------------------------------------------------------
   file= fopen(fileName, "w");
   if( file == NULL )
   {
     fprintf(stderr, "File(%s), ", fileName);
     perror("open failure");
     exit(EXIT_FAILURE);
   }

   //-------------------------------------------------------------------------
   // Function
   //-------------------------------------------------------------------------
   for(;;)                           // stdin => stdout + file
   {
     for(;;)                        // Read with retry recovery
     {
       errno= 0;                    // Default, no error
       size= fread(buff, 1, sizeof(buff), stdin);
       if( size > 0 || errno != EAGAIN )
         break;
     }

     if( size == 0 )                // If end of file
       break;                       // Done

     L= fwrite(buff, 1, size, stdout);
     if( L != size )
     {
       fprintf(stderr, "File(%s), ", "stdout");
       perror("I/O error");
       exit(EXIT_FAILURE);
     }

     L= fwrite(buff, 1, size, file);
     if( L != size )
     {
       fprintf(stderr, "File(%s), ", fileName);
       perror("I/O error");
       exit(EXIT_FAILURE);
     }
   }

   //-------------------------------------------------------------------------
   // Return
   //-------------------------------------------------------------------------
   return 0;
}
