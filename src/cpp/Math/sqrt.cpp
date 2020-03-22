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
//       sqrt.cpp
//
// Purpose-
//       Compute sqrt(x).
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

//----------------------------------------------------------------------------
//
// Subroutine-
//       newton
//
// Purpose-
//       Use newton's method to determine sqrt.
//
//----------------------------------------------------------------------------
static double                       // Return value
   newton(                          // Determine sqrt
     double          x)             // Of this value
{
   double            x1= x;         // Resultant
   double            x0;            // Working value

   if( x < 0.0 )
     return -1.0;

   //-------------------------------------------------------------------------
   // Evaluate
   //-------------------------------------------------------------------------
   int iteration= 0;
   double delta= x;
   x0= x;
   while( fabs(delta) > 0.0 )
   {
     double prior= delta;
     x1= x0 - (x0*x0 - x) / (2.0 * x0);
     delta= x1 - x0;
     x0= x1;
     iteration++;

     //-----------------------------------------------------------------------
     // Early exit if no progress after first iteration
//   printf("iteration(%d) delta(%.16g)\n", iteration, delta);
     if( fabs(delta) >= fabs(prior) && iteration > 1 )
       break;
   }

   return x1;
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
   double            x;             // The current x

   int               i;

   //-------------------------------------------------------------------------
   // Evaluate
   //-------------------------------------------------------------------------
   if( argc < 2 )
     printf("sqrt value ...\n");

   for(i= 1; i<argc; i++)
   {
     x= atof(argv[i]);
     printf("%.16g= sqrt(%.16g)\n", sqrt(x), x);
     printf("%.16g= newt(%.16g)\n", newton(x), x);
   }

   return 0;
}

