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
//       integrate.cpp
//
// Purpose-
//       Integrator
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
#define __SOURCE__       "INTEGRATE" // Source file

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define STEPS 512.0                  // Number of integration steps

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
static char   sw_path;              // Perform path integration
static char   sw_plot;              // Perform plot function

//----------------------------------------------------------------------------
//
// Subroutine-
//       f
//
// Purpose-
//       The function to be integrated
//
//----------------------------------------------------------------------------
#include "function.h"               // Function selector
#include "plot.h"                   // Plot function

//----------------------------------------------------------------------------
//
// Subroutine-
//       areaF
//
// Purpose-
//       Compute Area of f()
//
//----------------------------------------------------------------------------
static double                       // Resultant Area(f)
   areaF(                           // Area of f()
     double          lower,         // Lower bound
     double          delta)         // Increment
{
   double            resultant;     // Path of f()
   double            upper;         // Upper bound
   double            x;             // Function parameter
   double            y;             // Function resultant
   double            prior;         // Prior value

// printf("area: lower(%f) delta(%f)\n", lower, delta);
   if( delta > 0.0 )
     upper= lower + delta;
   else
   {
     if( delta == 0.0 )
       return 0.0;

     upper= lower;
     lower= lower - delta;
     delta= (-delta);
   }

   prior= f(lower);
   delta /= STEPS;
   resultant= 0.0;
// printf("area: lower(%f) upper(%f) delta(%f)\n", lower, upper, delta);
   for(x= lower+delta; x<upper; x += delta)
   {
     y= f(x);
     resultant += prior * delta;    // Rectangle base
     resultant += delta * (y-prior) * 0.5; // Triangle delta

     prior= y;
   }

   return resultant;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       pathF
//
// Purpose-
//       Compute Path of f()
//
//----------------------------------------------------------------------------
static double                       // Resultant Path(f)
   pathF(                           // Path of f()
     double          lower,         // Lower bound
     double          delta)         // Increment

{
   double            delta2;        // (Modified) delta * delta
   double            resultant;     // Path of f()
   double            upper;         // Upper bound
   double            x;             // Function parameter
   double            y;             // Function resultant
   double            prior;         // Prior value

// printf("path: lower(%f) delta(%f)\n", lower, delta);
   if( delta > 0.0 )
     upper= lower + delta;
   else
   {
     if( delta == 0.0 )
       return 0.0;

     upper= lower;
     lower= lower - delta;
     delta= (-delta);
   }

   prior= f(lower);
   delta /= STEPS;
   delta2= delta * delta;
   resultant= 0.0;
// printf("path: lower(%f) upper(%f) delta(%f)\n", lower, upper, delta);
   for(x= lower+delta; x<upper; x += delta)
   {
     y= f(x);
     prior -= y;
     resultant += sqrt((prior*prior) + delta2);

     prior= y;
   }

   return resultant;
}

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
   fprintf(stderr, "integrate {options}\n");
   fprintf(stderr, " -lower:value  Lower bound\n");
   fprintf(stderr, " -upper:value  Upper bound\n");
   fprintf(stderr, " -steps:value  Increment\n");
   fprintf(stderr, "\n");
   fprintf(stderr, " -help    Print this message\n");
   fprintf(stderr, " -path    path (rather than area) integration\n");
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

   sw_path= FALSE;
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

       else if ( swname("path", argp) )
         sw_path= swatob("path", argp);

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
     printf("\n");
     printf("%sF\n",  sw_path ? "path" : "area" );
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
   double            result;        // Integral
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
   // Integrate
   //-------------------------------------------------------------------------
   result= 0.0;
   delta= (upper - lower) / steps;
// printf("main: lower(%f) upper(%f) delta(%f)\n", lower, upper, delta);
   for(x=lower; x<=upper; x += delta)
   {
//   printf("%12.3e = integral(%s){%12.3e,%12.3e}\n", result, fName(), lower, x);
     if( sw_path )
       result += pathF(x, delta);
     else
       result += areaF(x, delta);

     if( sw_plot )
       plot(x);
   }
   printf("%12.3e = integral(%s){%12.3e,%12.3e}\n",
          result, fName(), lower, upper);

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   if( sw_plot )
     endPlot();

   return 0;
}

