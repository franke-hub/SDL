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
//       differ.cpp
//
// Purpose-
//       Differentiator
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
#define __SOURCE__       "DIFFATE  " // Source file

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define EPSILON 1.0e-8               // Small number

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
// Structures
//----------------------------------------------------------------------------
static char   sw_plot;              // Perform plot function

//----------------------------------------------------------------------------
//
// Subroutine-
//       f
//
// Purpose-
//       The function to be differentiated
//
//----------------------------------------------------------------------------
#include "function.h"               // Function selector

//----------------------------------------------------------------------------
//
// Subroutine-
//       dydx
//
// Purpose-
//       Differential dy/dx
//
//----------------------------------------------------------------------------
inline double                       // Resultant dy/dx
   dydx(                            // Differential dy/dx
     double          x)             // At x
{
   double            result;        // Resultant
   double            y;             // Function(x) resultant
   double            yPrime;        // Function(x+EPSILON) resultant

   y= f(x);
   yPrime= f(x+EPSILON);

   result= (yPrime-y)/EPSILON;
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       plot
//
// Purpose-
//       The plot function
//
//----------------------------------------------------------------------------
#define PLOTF dydx                  // Override default PLOTF
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
   fprintf(stderr, "differ {options}\n");
   fprintf(stderr, " -lower:value  Lower bound\n");
   fprintf(stderr, " -upper:value  Upper bound\n");
   fprintf(stderr, " -steps:value  Increment\n");
   fprintf(stderr, "\n");
   fprintf(stderr, " -help    Print this message\n");
   fprintf(stderr, " -plot    plot R\n");
   fprintf(stderr, " -verify  Verify parameters\n");
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
     char           *argv[])        // Argument array
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

     else if ( ! fParm(argv[argi]) ) // If invalid parameter
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
     char           *argv[])        // Argument array
{
   double            delta;         // The step factor
   double            x;             // The current x

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
   // Differentiate
   //-------------------------------------------------------------------------
   delta= (upper - lower) / steps;
// printf("main: lower(%f) upper(%f) delta(%f)\n", lower, upper, delta);
   for(x=lower+delta; x<=upper; x += delta)
   {
     printf("x(%12.6f) dy/dx(%12.6f)\n", x, dydx(x));

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

