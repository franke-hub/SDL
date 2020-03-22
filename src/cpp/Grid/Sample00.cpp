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
//       Sample00.cpp
//
// Purpose-
//       Generate raw height data for southwest region.
//
// Last change date-
//       2011/01/01
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

static const float     MIN_HEIGHT = 18.0; // Minimum allowed height
static const float     MAX_HEIGHT = 24.0; // Maximum allowed height
//atic const float     SEA_LEVEL  = 20.0; // Nominal zero height

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
   Position ORIGIN(0.0, 0.0, 0.0);
   Position HIGH_SPOT(255.0, 255.0, 0.0);
   float MAX_DISTANCE= ORIGIN.separation(HIGH_SPOT);

   for(float y= 0.0; y<256.0; y+=1.0)
   {
     for(float x= 0.0; x<256.0; x+=1.0)
     {
       XY p(x, y);
       float d= HIGH_SPOT.separation(p);
       d= MAX_DISTANCE - d;

       float h= MIN_HEIGHT;
       h += (MAX_HEIGHT-MIN_HEIGHT) * d/MAX_DISTANCE;
//     h -= SEA_LEVEL;

       if( x != 0.0 )
         printf(" ");
       printf("%9.6f", h);
     }
     printf("\n");
   }
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
   error("Sample00: Generate raw output file\n");
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

