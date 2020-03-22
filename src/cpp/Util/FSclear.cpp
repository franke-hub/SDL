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
//       FSclear
//
// Purpose-
//       FileSystem clear - fill up filesystem with random data
//
// Last change date-
//       2007/01/01
//
// Parameters-
//       Argc     = Argument count
//       Argv[0]  = Command name
//       Argv[1]  = Output file (default "ERASE.ME")
//
// Returns-
//       **None**
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#include <com/nativeio.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DATA_PATTERN          FALSE // Data patterns?

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define BUFSIZE               16384 // I/O buffer size
#define outfile             argv[1] // Output file name

//----------------------------------------------------------------------------
// Static constants
//----------------------------------------------------------------------------
#if DATA_PATTERN
static char            databyte[]= {  // Data byte values
   0xFF, 0xAA, 0x55, 0x00
   };
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     outname;     // Output file name
static char            buffer[BUFSIZE]; // I/O buffer

//----------------------------------------------------------------------------
//
// Subroutine-
//       randomize
//
// Function-
//       Set a random seed value.
//
//----------------------------------------------------------------------------
static void
   randomize( void )                // Set time-dependent random seed
{
   srand((unsigned)time(NULL));     // Time-dependent random seed
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       fsfill
//
// Function-
//       Fill filesystem with whatever's in buffer[].
//
//----------------------------------------------------------------------------
static void
   fsfill( void )                   // Fill filesystem
{
   int                 outh;        // File handles
   int                 outlen;      // Sizeof(buffer) this operation

   //-------------------------------------------------------------------------
   // Write the output file
   //-------------------------------------------------------------------------
   outh= open(outname,
              O_WRONLY|O_BINARY|O_TRUNC|O_CREAT,
              S_IREAD|S_IWRITE);    // Open for write
   if( outh < 0 )                   // If we cannot open the output file
   {
     printf ("Error, cannot create output file '%s'\n",outname);
     return;
   }

   while(1)                         // Create the output file
   {
     outlen= write(outh, buffer, BUFSIZE); // Write the buffer
     if( outlen != BUFSIZE )        // If output error
       break;                       // Exit, treat as disk full
   }

   while(1)                         // Create the output file
   {
     outlen= write(outh, buffer, BUFSIZE/2); // Write the buffer
     if( outlen != BUFSIZE/2 )      // If output error
       break;                       // Exit, treat as disk full
   }

   while(1)                         // Create the output file
   {
     outlen= write(outh, buffer, BUFSIZE/4); // Write the buffer
     if( outlen != BUFSIZE/4 )      // If output error
       break;                       // Exit, treat as disk full
   }

   while(1)                         // Create the output file
   {
     outlen= write(outh, buffer, BUFSIZE/8); // Write the buffer
     if( outlen != BUFSIZE/8 )      // If output error
       break;                       // Exit, treat as disk full
   }

   while(1)                         // Create the output file
   {
     outlen= write(outh, buffer, BUFSIZE/16); // Write the buffer
     if( outlen != BUFSIZE/16 )     // If output error
       break;                       // Exit, treat as disk full
   }

   while(1)                         // Create the output file
   {
     outlen= write(outh, buffer, BUFSIZE/32); // Write the buffer
     if( outlen != BUFSIZE/32 )     // If output error
       break;                       // Exit, treat as disk full
   }

   close(outh);                     // Close the file
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Function-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 outh;        // File handles
   int                 i;

#if DATA_PATTERN
   int                 datai;       // Databyte index
#endif

   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   randomize();                     // Initial random value
   outname= outfile;                // Default, outname is outfile
   if( argc <= 1 )                  // If no outfile specified
     outname= "ERASE.ME";           // Change name to default

   //-------------------------------------------------------------------------
   // File initialization
   //-------------------------------------------------------------------------
   outh= open(outname, O_RDONLY|O_BINARY, 0);// Open for read
   if( outh >= 0 )                  // If the output file already exists
   {
     printf ("Error, output file '%s' exists.\n",outname);
     return 1;
   }

#if DATA_PATTERN
   //-------------------------------------------------------------------------
   // Fill the file with pattern data
   //-------------------------------------------------------------------------
   for(datai=0; datai<sizeof(databyte); datai++)
   {
     for(i=0; i<BUFSIZE; i++)
       buffer[i]= databyte[datai];

     fsfill();
     printf("Pattern 0x%.2X complete\n", databyte[datai]);
   }
#endif

   //-------------------------------------------------------------------------
   // Fill the file with random data
   //-------------------------------------------------------------------------
   for(i=0; i<BUFSIZE; i++)         // Initialize the buffer
     buffer[i]= rand();             // Set random value
   fsfill();
   printf("Pattern RAND complete\n");
   return 0;
}

