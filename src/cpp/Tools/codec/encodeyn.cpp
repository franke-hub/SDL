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
//       encodeyn.cpp
//
// Purpose-
//       Convert a file into its yEnc encoding.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include <com/params.h>
#include <com/FileInfo.h>
#include <com/Reader.h>
#include <com/Writer.h>

#include "YncodeCodec.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

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
   fprintf(stderr, "encodeyn filename ... >output-filename\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "filename ...\n");
   fprintf(stderr, "  The list of files to encode\n");
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

   int                 count;       // The number of files to compare
   int                 error;       // Error encountered indicator
// int                 verify;      // Verification control

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   count= 0;                        // No files found yet
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
       count++;
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( count < 1 )                  // If too few files specified
   {
     error= TRUE;
     fprintf(stderr, "No filename specified\n");
   }

   if( error )                      // If error encountered
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       encodeyn
//
// Purpose-
//       yEnc encode a file into STDOUT.
//
//----------------------------------------------------------------------------
int                                 // Return code
   encodeyn(                        // yEnc encode a file
     const char*       fileName)    // -> FileName to encode
{
   int                 result;

   YncodeCodec         codec;       // Codec object
   FileInfo            info(fileName); // Source info
   FileReader          inp;         // Source file
   FileWriter          out;         // Target file (stdout)
   uint64_t            size= info.getFileSize(); // Source file size

   //-------------------------------------------------------------------------
   // Open the files
   //-------------------------------------------------------------------------
   inp.open(fileName);              // Open the file
   out.open(NULL);                  // (stdout)

   //-------------------------------------------------------------------------
   // Write the header
   //-------------------------------------------------------------------------
   out.printf("=ybegin line=128 size=%" PRIu64 " name=%s\n", size, fileName);

   //-------------------------------------------------------------------------
   // Decode the file
   //-------------------------------------------------------------------------
   result= codec.encode(inp, out);

   //-------------------------------------------------------------------------
   // Write the trailer
   //-------------------------------------------------------------------------
   out.printf("=yend size=%" PRIu64 " crc32=%.8x\n", size, codec.getSum());
   inp.close();

   return result;
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
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 rc;          // Called routine's return code
   int                 returncd;    // This routine's return code

   int                 i;

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   returncd= 0;
   for(i=1; i<argc; i++)
   {
     if( argv[i][0] == '-' )        // If this parameter is in switch format
       continue;

     try {
       rc= encodeyn(argv[i]);       // Encode from file
       if( rc != 0 )                // If failure
         returncd= 1;               // Indicate it
     } catch(const char* X) {
       fprintf(stderr, "Exception: %s\n", X);
     } catch(...) {
       fprintf(stderr, "Exception: ...\n");
     }
   }

   //-------------------------------------------------------------------------
   // Return
   //-------------------------------------------------------------------------
   return returncd;
}

