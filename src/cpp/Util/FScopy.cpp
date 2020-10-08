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
//       FScopy.cpp
//
// Purpose-
//       Copy files (with partial recovery for missing blocks.)
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <com/define.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "FSCOPY  " // File name, for messages

#ifndef TEST_SLOWLOAD
#undef  TEST_SLOWLOAD
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     inpName;     // The input file name
static const char*     outName;     // The output file name
static unsigned long   outSize;     // The output buffer length
static char*           outBuff;     // The output buffer
static char*           outOrig;     // The output buffer origin

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
   fprintf(stderr, "copy inp-filename out-filename\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "inp-filename\t(The input file name)\n");
   fprintf(stderr, "out-filename\t(The output file name)\n");
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

   int                 error;       // Error encountered indicator
// int                 verify;      // Verification control

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   error= FALSE;                    // Set defaults
// verify= FALSE;
   inpName= outName= NULL;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If this parameter is in switch format
     {
//     if( strcmp("-verify", argp) == 0 )// If verify switch
//       verify= TRUE;              // (Currently ignored!)
//
//     else                         // If invalid switch
       {
         error= TRUE;
         fprintf(stderr, "Invalid parameter '%s'\n", argv[argi]);
       }
     }
     else                           // If filename parameter
     {
       if( inpName == NULL )
         inpName= argp;

       else if( outName == NULL )
         outName= argp;

       else
       {
         error= TRUE;
         fprintf(stderr, "Unexpected file name '%s'\n", argv[argi]);
       }
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( outName == NULL )            // If too few files specified
   {
     error= TRUE;
     if( inpName == NULL )
       fprintf(stderr, "No filenames specified\n");
     else
       fprintf(stderr, "Missing output filename\n");
   }

   if( error )                      // If error encountered
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   init( void )                     // Initialize
{
   unsigned            size;        // Allocation size
   struct stat         st;          // File status area

   int                 rc;

   rc= stat(outName, &st);          // Get file information
   if( rc == 0 )                    // If not found
   {
     fprintf(stderr, "File(%s) exists\n", outName);
     return 1;
   }

   rc= stat(inpName, &st);          // Get file information
   if( rc != 0 )                    // If not found
   {
     fprintf(stderr, "File(%s) ", inpName);
     perror("stat");
     return 1;
   }

   outSize= st.st_size;             // Get the size
   if( (off_t)outSize != st.st_size ) // If more than 32 bits
   {
     fprintf(stderr, "File(%s) too large (%" PRId64 ")", inpName, (uint64_t)st.st_size);
     return 1;
   }

   size= outSize + 8192;            // Round up
   size &= (-4096);                 // Truncate down
   outOrig= (char*)malloc(size);    // Allocate the buffer
   if( outOrig == NULL )
   {
     fprintf(stderr, "No storage(%lu)", outSize);
     return 1;
   }
   outBuff= (char*)(((size_t)outOrig + 4095) & (-4096));

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       Terminate
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   term( void )                     // Terminate
{
   free(outBuff);
   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       fastLoad
//
// Purpose-
//       Fastpath (no error) loader.
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   fastLoad( void )                 // Load the file
{
   FILE*               file;        // File handle
   unsigned            L;           // Actual length
   unsigned            offset;      // Current offset
   unsigned            size;        // Desired length

   int                 rc;

   //-------------------------------------------------------------------------
   // Open the file
   //-------------------------------------------------------------------------
   file= fopen(inpName, "rb");      // Open the input file
   if( file == NULL )
   {
     fprintf(stderr, "File(%s) ", inpName);
     perror("open failure");
     return 2;
   }

   //-------------------------------------------------------------------------
   // Read the file
   //-------------------------------------------------------------------------
   offset= 0;
   for(;;)                          // Read the file
   {
     size= outSize - offset;        // Remaining length
     L= fread(outBuff+offset, 1, size, file);
     if( L == size )
       break;

     if( ferror(file) || int(L) < 0 )
     {
       fprintf(stderr, "File(%s) ", inpName);
       perror("read failure");
       fclose(file);
       return 1;
     }

     if( L == 0 )
     {
       fprintf(stderr, "File(%s) ", inpName);
       perror("premature EOF");
       fclose(file);
       return 1;
     }

     offset += L;
   }

   //-------------------------------------------------------------------------
   // Close the file
   //-------------------------------------------------------------------------
   rc= fclose(file);                // Close the file
   if( rc != 0 )
   {
     fprintf(stderr, "File(%s) ", inpName);
     perror("close failure");
     rc= 1;
   }

   #ifdef TEST_SLOWLOAD
     rc= 1;
   #endif
   return rc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       slowLoad
//
// Purpose-
//       Slowpath (error handling) loader.
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   slowLoad( void )                 // Load the file
{
   FILE*               file;        // File handle
   unsigned            L;           // Actual length
   unsigned            offset;      // Current offset
   unsigned            size;        // Desired length
   unsigned            state;       // Current state (0 normal)

   int                 rc;

   //-------------------------------------------------------------------------
   // Open the file
   //-------------------------------------------------------------------------
   file= fopen(inpName, "rb");      // Open the input file
   if( file == NULL )
   {
     fprintf(stderr, "File(%s) ", inpName);
     perror("open failure");
     return 2;
   }
   printf("File(%s) error recovery\n", inpName);
   printf("OK open()\n");

   //-------------------------------------------------------------------------
   // Read the file
   //-------------------------------------------------------------------------
   offset= 0;
   state= 0;
   while( offset < outSize )        // Write the file
   {
     //-----------------------------------------------------------------------
     // Set read length:
     // Maximum= remaining length
     // Normal= 4096
     // Error= 512;
     size= outSize - offset;        // Maximum read length
     if( size > 4096 && (offset&4095) == 0 )
       size= 4096;
     if( state != 0 && size > 512 )
       size= 512;

     //-----------------------------------------------------------------------
     // Set position.
     rc= fseek(file, offset, SEEK_SET);
     if( rc != 0 )
     {
       fprintf(stderr, "NG %10lu ", (long)offset);
       perror("fseek failure");
     }

     //-----------------------------------------------------------------------
     // Write the block.
     L= fread(outBuff+offset, 1, size, file);

     //-----------------------------------------------------------------------
     // Handle the block.
     if( ferror(file) )
     {
       L= size;
       if( state == 0 )
       {
         L= 0;
         state= 1;
       }

       clearerr(file);
     }
     else
     {
       state= 0;
       if( L == 0 )
       {
         printf("NG %10lu EOF(%lu)\n", (long)offset, (long)outSize);
         break;
       }
     }

     printf("%s %10lu.%.4d\r", state == 0 ? "OK" : "NG", (long)offset, size);
     offset += L;
   }
   printf("\n");

   //-------------------------------------------------------------------------
   // Close the file
   //-------------------------------------------------------------------------
   rc= fclose(file);                // Close the file
   if( rc != 0 )
   {
     fprintf(stderr, "File(%s) ", inpName);
     perror("close failure");
     rc= 1;
   }

   return rc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       writer
//
// Purpose-
//       Write the file
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   writer( void )                   // Write the output file
{
   FILE*               file;        // File handle
   unsigned            L;           // Actual length
   unsigned            offset;      // Current offset
   unsigned            size;        // Desired length

   int                 rc;

   //-------------------------------------------------------------------------
   // Open the file
   //-------------------------------------------------------------------------
   file= fopen(outName, "wb");      // Open the output file
   if( file == NULL )
   {
     fprintf(stderr, "File(%s) ", outName);
     perror("open failure");
     return 1;
   }

   //-------------------------------------------------------------------------
   // Write the file
   //-------------------------------------------------------------------------
   offset= 0;
   for(;;)                          // Write the file
   {
     size= outSize - offset;        // Remaining length
     L= fwrite(outBuff+offset, 1, size, file);
     if( L == size )
       break;

     if( ferror(file) || L <= 0 )
     {
       fprintf(stderr, "File(%s) ", outName);
       perror("write failure");
       fclose(file);
       return 1;
     }

     offset += L;
   }

   //-------------------------------------------------------------------------
   // Close the file
   //-------------------------------------------------------------------------
   rc= fclose(file);                // Close the file
   if( rc != 0 )
   {
     fprintf(stderr, "File(%s) ", outName);
     perror("close failure");
   }

   return rc;
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
   int                 rc;          // Return code and resultant

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   rc= init();
   if( rc == 0 )
   {
     rc= fastLoad();                // Fastpath load
     if( rc == 1 )                  // If recoverable error
       rc= slowLoad();              // Load with recovery

     if( rc == 0 )                  // If file read
       writer();                    // Write it out
   }

   if( rc == 0 )
     rc= term();

   //-------------------------------------------------------------------------
   // Return
   //-------------------------------------------------------------------------
   return rc;
}

