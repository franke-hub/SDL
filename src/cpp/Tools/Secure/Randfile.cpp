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
//       Randfile.cpp
//
// Purpose-
//       Generate random file.
//
// Last change date-
//       2007/01/01                 Version 2, Release 1
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>                 // For int32_t
#include <stdlib.h>
#include <sys/stat.h>

#include <com/nativeio.h>
#include <com/sysmac.h>             // For min()
#include "Crypto.h"

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#undef random
#define random(x) (myrand()%x)

//----------------------------------------------------------------------------
// I/O control area
//----------------------------------------------------------------------------
static int             outh;        // Output file handle
static int             outlen;      // Current size of buffer
static int             outndx;      // Current index for buffer
static const char*     outname;     // Pointer to file name
static char            outbuf[OUTSIZE]; // The I/O buffer itself

//----------------------------------------------------------------------------
// General work areas
//----------------------------------------------------------------------------
static uint32_t        lastrand= 0; // Last random value

//----------------------------------------------------------------------------
//
// Subroutine-
//       MYRAND
//
// Function-
//       Random number generator.
//
//----------------------------------------------------------------------------
static
   int32_t myrand( void )
{
   int32_t             work;        // Working variable

   if( lastrand == 0 )
     lastrand= 4095;

   work=(7789*lastrand)%131071L;

   lastrand= work;
   return(work);                    // Return random number
}

//----------------------------------------------------------------------------
//
// Subroutine-
//      parmwd
//
// Function-
//      Extract parameter word value.
//
//----------------------------------------------------------------------------
static Word                         // Resultant
   parmwd(                          // Parameter word value
     char*             param)       // -> String
{
   Word                resultant= 0;// The parameter word resultant

   while( *param )                  // Process each character
   {
     resultant= lrot(resultant, 6); // Position the resultant
     resultant += *(signed char*)param; // Add the character value
     param++;                       // Address next character
   }

   return(resultant);               // Return, function complete
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       openout
//
// Function-
//       Open the output file.
//
//----------------------------------------------------------------------------
static void
   openout( void )                  // Open the output file
{
   outh= open(outname, O_RDONLY|O_BINARY, 0);// Open the output file (for input)
   if( outh >= 0 )                  // If we can open the file, it must exist
   {
     printf ("Error, output file '%s' exists.\n",outname);
     close(outh);
     exit(EXIT_FAILURE);
   }

   outh= open(outname,              // Open the output file
              O_WRONLY|O_BINARY|O_TRUNC|O_CREAT,// (in write-only binary mode)
              S_IREAD|S_IWRITE);    // (with full write access)
   if( outh < 0 )                   // If the open failed
   {
     printf ("Error, cannot create output file '%s'\n",outname);
     exit(EXIT_FAILURE);
   }

   outndx= 0;                       // Now at top of buffer
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       wfinis
//
// Function-
//       Flush, close the output file.
//
//----------------------------------------------------------------------------
static void
   wfinis( void )                   // Write, the close output file
{
   int                 rc;          // Return code

   rc= close(outh);                 // Close the output file
   if( rc != 0 )                    // If close failure
   {
     printf ("Error writing file '%s'.\n",outname);
     return;
   }
}

//----------------------------------------------------------------------------
//
// Segment-
//       RANDFILE
//
// Function-
//       Mainline code.
//
//----------------------------------------------------------------------------
int
   main(                            // Encryption/Decryption routine
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 i;           // General index variable
   int                 l;           // Current write length
   long int            fsize;       // File size
   long int            csize;       // Current size
   uint32_t            xlate;       // Translation word
   unsigned char       xchar[4];    // Translation string

   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   fsize= 0;
   if( argc >= 2 )
     fsize= atol(argv[1]);

   if( fsize <= 0 )
   {
     printf("Parameter error\n");
     exit(EXIT_FAILURE);
   }

   //-------------------------------------------------------------------------
   // Key initialization
   //-------------------------------------------------------------------------
   xlate= 0;                        // Set last encryption key value
   if( argc > 2 )                   // If initial key specified
   {
     for(i=2; i<argc; i++)          // Set initial seed value
     {
       xlate ^= lrot(xlate, 5);     // Randomize previous value
       xlate += parmwd(argv[i]);    // Add key value
       xlate += lrot(xlate, 27);    // Randomize previous value
     }

#if INSTRUMENT_KEYWORD              // If Key instrumentation
     printf("Extended key:");
     for(i=2; i<argc; i++)
       printf(" %s", argv[i]);
     printf("\n");
#endif                              // INSTRUMENT_KEYWORD

#if INSTRUMENT_KEYCODE              // If Key instrumentation
     printf("Extended code: 0x%.8lX\n", (long)xlate);
#endif                              // INSTRUMENT_KEYCODE
   }

   xchar[0]= (xlate & 0xFF000000L) >> 24;
   xchar[1]= (xlate & 0x00FF0000L) >> 16;
   xchar[2]= (xlate & 0x0000FF00L) >>  8;
   xchar[3]= (xlate & 0x000000FFL)      ;

   //-------------------------------------------------------------------------
   // Create the output file
   //-------------------------------------------------------------------------
   outname= "rfo";
   openout();                       // Open the output file
   csize= 0;                        // Current file size

   while( csize < fsize )
   {
     for(i=0; i < OUTSIZE; i++)
       outbuf[i]= random(256);

     for(i=0; i < OUTSIZE; i++)
       outbuf[i] ^= xchar[(i&3)];

     outlen= min(OUTSIZE, fsize - csize);
     l= write(outh, outbuf, outlen);
     if( l != outlen )
     {
       printf("Error writing '%s', file not usable\n", outname);
       close(outh);
       exit(EXIT_FAILURE);
     }

     csize += OUTSIZE;
   }

   //-------------------------------------------------------------------------
   // Handle the file trailer
   //-------------------------------------------------------------------------
   wfinis();                        // Flush, close the output file
   return 0;
}

