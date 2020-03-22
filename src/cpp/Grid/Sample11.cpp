//----------------------------------------------------------------------------
//
//       Copyright (c) 2011 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Sample11.cpp
//
// Purpose-
//       Generate raw height data for volcano region.
//
// Last change date-
//       2011/01/01
//
// Implementation data-
//       25 30 25
//       30 HH 30
//       25 30 25
//
//----------------------------------------------------------------------------
#include <stdarg.h>                 // For va_list
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp
#include <sys/stat.h>               // For struct stat

#include "Position.h"

//----------------------------------------------------------------------------
// Constant for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

static const float     MIN_HEIGHT =  24.0; // Minimum allowed height
static const float     MAX_HEIGHT = 120.0; // Maximum allowed height
static const float     LOWER_DIAM =  30.0; // Inner volcano diameter (MIN_HEIGHT)
static const float     UPPER_DIAM =  32.0; // Upper volcano diameter (MAX_HEIGHT)
static const float     OUTER_DIAM =  50.0; // Outer volcano diameter (MAX_HEIGHT)

// Computed constants
static const Position  ORIGIN(  0.0,   0.0, MIN_HEIGHT);
static const Position  CENTER(128.0, 128.0, MIN_HEIGHT);
static const float     MAX_DISTANCE= ORIGIN.separation(CENTER) - OUTER_DIAM;
static const float     DEL_HEIGHT= MAX_HEIGHT-MIN_HEIGHT;

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug
//
// Purpose-
//       Write a message onto stderr
//
//----------------------------------------------------------------------------
static void
   debug(                           // Write error message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   if( TRUE  )
   {
     va_list             argptr;    // Argument list pointer

     va_start(argptr, fmt);         // Initialize va_ functions
     vfprintf(stderr, fmt, argptr);
     va_end(argptr);                // Close va_ functions
   }
}

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
//       genHeight
//
// Purpose-
//       Generate height for position.
//
//----------------------------------------------------------------------------
static float                        // Resultant height
   genHeight(                       // Generate height
     float             x,           // For this X
     float             y)           // And this Y position
{
   XY    p(x,y);                    // For this position
   float d= CENTER.separation(p);
   float h= 0.0;                    // Resultant height

   if( d < LOWER_DIAM )
     h= MIN_HEIGHT;
   else if( d < UPPER_DIAM )
   {
     float deltaD= d - LOWER_DIAM;
     h= MIN_HEIGHT + deltaD*DEL_HEIGHT/(UPPER_DIAM-LOWER_DIAM);
   }
   else if( d < OUTER_DIAM )
     h= MAX_HEIGHT;
   else
   {
     d -= OUTER_DIAM;
     float deltaD= MAX_DISTANCE - d;
     deltaD *= deltaD;

     h= MIN_HEIGHT + deltaD*DEL_HEIGHT/(MAX_DISTANCE*MAX_DISTANCE);
   }

   return h;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       generate
//
// Purpose-
//       Generate the raw format output file.
//
// Implementation notes-
//       Height inversely proportional to distance from HIGH_SPOT.
//
//----------------------------------------------------------------------------
static void
   generate( void )                 // Generate the output file
{

   for(float y= 0.0; y<256.0; y+=1.0)
   {
     for(float x= 0.0; x<256.0; x+=1.0)
     {
       float h= genHeight(x,y);

       if( x != 0.0 )
         printf(" ");
       printf("%9.4f", h);
     }
     printf("\n");
   }

   debug("%9.6f= genHeight(%f,%f)\n", genHeight(0.0,  0.0), 0.0,   0.0);
   debug("%9.6f= genHeight(%f,%f)\n", genHeight(0.0, 32.0), 0.0,  32.0);
   debug("%9.6f= genHeight(%f,%f)\n", genHeight(0.0, 64.0), 0.0,  64.0);
   debug("%9.6f= genHeight(%f,%f)\n", genHeight(0.0, 96.0), 0.0,  96.0);
   debug("%9.6f= genHeight(%f,%f)\n", genHeight(0.0,128.0), 0.0, 128.0);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Display program information
{
   error("Sample11: Generate raw output file\n");
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
   if( FALSE )
     info();

   generate();

   return EXIT_SUCCESS;
}

