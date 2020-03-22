//----------------------------------------------------------------------------
//
//       Copyright (c) 2011-2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Raw2Map.cpp
//
// Purpose-
//       Graphical raw floating point file mapping.
//
// Last change date-
//       2012/01/01
//
// Input-
//       stdin: A raw floating point file, 256x256.
//         (Terminal input not supported.)
//
// Output-
//       Graphic map of values.
//
//----------------------------------------------------------------------------
#include <stdarg.h>                 // For va_list
#include <math.h>                   // For sqrt
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp
#include <sys/stat.h>               // For struct stat

#include <gui/Types.h>
#include <gui/Object.h>
#include <gui/Window.h>
using namespace GUI;

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static float           height[256][256]; // The input data array
static float           MAX_HEIGHT;  // Maximum height
static float           MIN_HEIGHT;  // Minimum height

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
//       info
//
// Purpose-
//       Parameter fault exit.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter fault exit
{
   error("Raw2Map filename\n\n"
         "Converts a raw input file into a graphic map.\n"
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

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If this parameter is in switch format
     {
       if( strcmp("-help", argp) == 0 )  // If help request
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
       ERROR= TRUE;
       error("Unexpected file name '%s'\n", argv[argi]);
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( !HELPI )
   {
     struct stat s;
     int fn= fileno(stdin);
     int rc= fstat(fn, &s);
     if( rc != 0 )
     {
       ERROR= TRUE;
       error("ERROR: %d= stat(STDIN), ", rc);
       perror("stat(STDIN)");
     }
   }

   if( HELPI || ERROR )
   {
     if( ERROR )
       error("\n");

     info();
   }

   if( verify )
   {
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       load
//
// Purpose-
//       Load the raw input file
//
//----------------------------------------------------------------------------
static void
   load( void )                     // Load the raw input file
{
   int inpSize= 256;
   float* line= (float*)malloc(inpSize * sizeof(float));
   for(int i= 0; i<256; i++)
   {
     int L= fread(line, sizeof(float), inpSize, stdin);
     if( L <= 0 )
       break;

     if( L != inpSize )
     {
       error("Error: File(STDIN), read error(%d) %d\n", L, inpSize);
       break;
     }

     for(int j= 0; j<256; j++)
       height[j][i]= line[j];
   }

   MAX_HEIGHT= height[0][0];
   MIN_HEIGHT= height[0][0];
   for(int i= 0; i<256; i++)
   {
     for(int j= 0; j<256; j++)
     {
       if( height[i][j] > MAX_HEIGHT )
         MAX_HEIGHT= height[i][j];

       if( height[i][j] < MIN_HEIGHT )
         MIN_HEIGHT= height[i][j];
     }
   }

   free(line);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       h2c
//
// Purpose-
//       Convert height to Color_t
//
//----------------------------------------------------------------------------
static Color_t                      // Resultant color
   h2c(                             // Convert height to color
     float             h)           // Height
{
   Color_t result= 0x0000000;

   if( h > 16.0 )                   // Below this is black
   {
     if( h < 20.0 )                 // Black to light blue
     {
       float f= (h - 16.0) / 4.0;
       f *= 256.0;
       int c= (int)f;

       result= c;
     }
     else if( h < 30.0 )            // Light green to red
     {
       float f= (h - 20.0) / 10.0;
       f *= 256.0;
       int c= (int)f;
       int i= 255 - c;

       result= (c<<16) | (i<<8);
     }
     else if( h < 40.0 )            // Red to yellow
     {
       float f= (h - 30.0) / 10.0;
       f *= 256.0;
       int c= (int)f;

       result= 0x00ff0000 | (c<<8);
     }
     else if( h < 140.0 )           // Yellow to white
     {
       float f= (h - 40.0) / 100.0;
       f *= 256.0;
       int c= (int)f;

       result= 0x00ffff00 | c;
     }
     else                           // White
       result= 0x00ffffff;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show
//
// Purpose-
//       Display the input data
//
//----------------------------------------------------------------------------
static void
   show( void )                     // Display the raw data
{
   XYLength length= {256,256};      // The window size (in Pixels)
   Window window(length);           // Create the window

   for(int i= 0; i<256; i++)
   {
     XOffset_t x= i;
     for(int j= 0; j<256; j++)
     {
       YOffset_t y= 255-j;
       Pixel* pixel= window.getPixel(x,y);
       pixel->setColor(h2c(height[i][j]));
     }
   }

   if( FALSE )
   {
     for(int i= 0; i<256; i++)
     {
       window.getPixel(0,i)->setColor(0x00ff0000);
       window.getPixel(255,i)->setColor(0x00ff0000);
       window.getPixel(i,255)->setColor(0x00ff0000);
       window.getPixel(i,0)->setColor(0x00ff0000);
     }
   }

   if( FALSE )
   {
     for(int i= 0; i<256; i++)
     {
       for(int j= 0; j<256; j++)
       {
         if( j != 0 )
           printf(" ");
         printf("%f", height[i][j]);
       }

       printf("\n");
     }
     fclose(stdout);
   }

   window.setAttribute(Object::VISIBLE, TRUE);
   window.change();
   window.wait();

   // For destructor, make invisible
   window.setAttribute(Object::VISIBLE, FALSE);
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
   load();                          // Load the data
   show();                          // Show the data

   return EXIT_SUCCESS;
}

