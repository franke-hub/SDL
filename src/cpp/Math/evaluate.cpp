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
//       evaluate.cpp
//
// Purpose-
//       Evaluate a function.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/params.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__        "EVALUATE" // Source file

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef M_PI
#define M_PI  3.14159265358979323846264338327950288
#endif

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static char          sw_plot;       // TRUE if plot

//----------------------------------------------------------------------------
//
// Subroutine-
//       f
//
// Purpose-
//       The function to be evaluated.
//
//----------------------------------------------------------------------------
#include "function.h"               // Function selector
#include "plot.h"                   // Plot function

//----------------------------------------------------------------------------
//
// Subroutine-
//       infoExit
//
// Purpose-
//       Display parameters and exit.
//
//----------------------------------------------------------------------------
static void
   infoExit( void )                 // Display parameters
{
   fprintf(stderr, "evaluate {options}\n");
   fprintf(stderr, "y= %s\n", fName());
   fprintf(stderr, " -help    Print this message\n");
   fprintf(stderr, " -plot    plot\n");
   fprintf(stderr, " -verify  Verify parameters\n");
   fprintf(stderr, "\n");
   fprintf(stderr, " -lower:value  Lower bound\n");
   fprintf(stderr, " -upper:value  Upper bound\n");
   fprintf(stderr, " -steps:value  Number of steps\n");
   fInfo();

   exit( EXIT_FAILURE );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parms
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static void
   parms(                           // Parameter analysis
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   char*             argp;          // Argument pointer
   int               argi;          // Argument index
   int               error= FALSE;  // Error encountered indicator
   int               verify= FALSE; // Verify indicator

   sw_plot= FALSE;

   errno= 0;
   for(argi=1; argi<argc; argi++)
   {
     argp= argv[argi];              // Address the parameter
     if ( *argp == '-' )            // If this is a switch
     {
       argp++;                      // Skip over the switch character
       if ( swname("help", argp) )
         infoExit();

       if ( swname("verify", argp) )
         verify= swatob("verify", argp);

       else if ( swname("plot", argp) )
         sw_plot= swatob("plot", argp);

       else if ( swname("lower:", argp) )
         lower=  swatod("lower:", argp);

       else if ( swname("upper:", argp) )
         upper=  swatod("upper:", argp);

       else if ( swname("scale:", argp) )
         scale=  swatod("scale:", argp);

       else if ( swname("steps:", argp) )
         steps=  swatod("steps:", argp);

       else if ( ! fParm(argv[argi]) ) // If invalid parameter
       {
         error= TRUE;
         fprintf(stderr, "Invalid parameter(-%s)\n", argp);
       }

       if( errno != 0 )
       {
         error= TRUE;
         fprintf(stderr, "-%s errno(%d) ", argp, errno);
         perror("");
         errno= 0;
       }
     }

     else if ( ! fParm(argv[argi]) )// If invalid parameter
     {
       error= TRUE;
       fprintf(stderr, "Invalid parameter(%s)\n", argp);
     }
   }

   if( error )
     infoExit();

   if( verify )
   {
     printf("%12.3e = lower\n",  lower);
     printf("%12.3e = upper\n",  upper);
     printf("%12.3e = steps\n",  steps);
     fShow();
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
extern int                          // Return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   double            delta;         // The step factor
   double            x;             // The current x
   double            y;             // Function(x) resultant

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parms(argc, argv);               // Parameter analysis

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   printf("y= %s\n", fName());
   if( sw_plot )
     prePlot();

   //-------------------------------------------------------------------------
   // Evaluate
   //-------------------------------------------------------------------------
   delta= (upper - lower) / steps;
   for(x=lower; x<=upper; x += delta)
   {
     y= f(x);
     printf("x(%12.6f) y(%12.6f)\n", x, y);

     if( sw_plot )
       plot(x);
   }

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   if( sw_plot )
     endPlot();

   return 0;
}

