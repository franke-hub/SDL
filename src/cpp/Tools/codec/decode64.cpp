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
//       decode64.cpp
//
// Purpose-
//       Decode a base64 encoded file.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#include <com/define.h>
#include <com/Buffer.h>
#include <com/istring.h>
#include <com/params.h>
#include <com/Reader.h>
#include <com/Unconditional.h>
#include <com/Writer.h>

#include "Base64Codec.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define LINE_SIZE             32768 // Allocated line size

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
   fprintf(stderr, "decode64 filename <input-filename\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "filename\n");
   fprintf(stderr, "  The output file name\n");
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
       if( count > 1 )
       {
         error= TRUE;
         fprintf(stderr, "Extra filename(%s) specified\n", argp);
       }
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
//       decode64
//
// Purpose-
//       Base64 decode STDIN into a file.
//
//----------------------------------------------------------------------------
int                                 // Return code
   decode64(                        // Base64 decode a file
     const char*       fileName)    // -> Target filename
{
   int                 result;

   Base64Codec         codec;       // Codec object
   FileReader          inp;         // Source file (stdin)
   FileWriter          out;         // Target file
   TempBuffer          temp;        // Extracted file
   int                 rc;

   //-------------------------------------------------------------------------
   // Open the files
   //-------------------------------------------------------------------------
   rc= inp.open(NULL);              // (stdin)
   if( rc != 0 )
   {
     fprintf(stderr, "File(<stdin) ");
     perror("Open failure");
     return 1;
   }

   rc= out.open(fileName);          // Open the file
   if( rc != 0 )
   {
     fprintf(stderr, "File(%s) ", fileName);
     perror("Open failure");
     return 1;
   }

   rc= temp.open(fileName, Media::MODE_WRITE);
   if( rc != 0 )
   {
     fprintf(stderr, "%4d: File(%s) TEMP open[WR] failure(%d)\n",
                     __LINE__, fileName, rc);
     return 1;
   }

   //-------------------------------------------------------------------------
   // Extract the header/trailer
   //-------------------------------------------------------------------------
   char* inpLine= (char*)Unconditional::malloc(LINE_SIZE);

   // Find the start delimiter
   for(;;)
   {
     rc= inp.readLine(inpLine, LINE_SIZE);
     if( rc == Reader::RC_SKIP )    // If overlength, ignore
       continue;

     if( rc < 0 )                   // End of file or error
     {
       free(inpLine);
       return Codec::DC_NOH;
     }

     if( memcmp(inpLine, "beg64 ", 6) == 0
         && inpLine[6] >= '0' && inpLine[6] <= '7'
         && inpLine[7] >= '0' && inpLine[7] <= '7'
         && inpLine[8] >= '0' && inpLine[8] <= '7'
         && inpLine[9] == ' ' )
       break;
   }

   // Copy into temporary
   for(;;)
   {
     rc= inp.readLine(inpLine, LINE_SIZE);
     if( rc < 0 )
       break;

     // Handle empty line
     if( inpLine[0] == '\0' )
       continue;

     // Handle "end" line
     if( stricmp("end", inpLine) == 0 )
       break;

     // Load the data line
     temp.printf("%s\n", inpLine);
   }

   free(inpLine);
   temp.close();

   //-------------------------------------------------------------------------
   // Decode the file
   //-------------------------------------------------------------------------
   rc= temp.open(fileName, Media::MODE_READ);
   if( rc != 0 )
   {
     fprintf(stderr, "%4d: File(%s) TEMP open[RD] failure(%d)\n",
                     __LINE__, fileName, rc);
     return 1;
   }

   result= codec.decode(temp, out);
   out.close();

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

     rc= decode64(argv[i]);         // Decode into file
     if( rc != 0 )                  // If failure
     {
       fprintf(stderr, "Decode failed(%d): %s\n", rc, argv[i]);
       returncd= 1;                 // Indicate it
     }
   }

   //-------------------------------------------------------------------------
   // Return
   //-------------------------------------------------------------------------
   return returncd;
}

