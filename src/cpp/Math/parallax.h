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
//       parallax.h
//
// Purpose-
//       Define the function: sqrt((3.0-.75*x)^2 + (2.0-.20*x)^2)
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef FUNCTION_H_INCLUDED
#define FUNCTION_H_INCLUDED

//----------------------------------------------------------------------------
//
// Segment-
//       Default values
//
// Purpose-
//       Define the default parameters
//
//----------------------------------------------------------------------------
static double        lower= 0.0;    // Evaluate lower
static double        upper= 10.0;   // Evaluate upper
static double        steps= 128.0;  // Evaluation steps

//----------------------------------------------------------------------------
//
// Subroutine-
//       f
//
// Purpose-
//       The function to be evaluated.
//
//----------------------------------------------------------------------------
inline double                       // Resultant (y)
   f(                               // The function
     double          x)             // Argument  (x)
{
   double            y0, y1;

   y0= 3.0 - .75*x;
   y1= 2.0 - .20*x;
   return sqrt(y0*y0 + y1*y1);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       fName
//
// Purpose-
//       The name of the function to be Evaluated
//
//----------------------------------------------------------------------------
const char*                         // The function name
   fName( void )                    // The function's name
{
   return "sqrt((3.0-.75*x)^2 + (2.0-.20*x)^2)";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       fInfo
//
// Purpose-
//       Display local parameter information.
//
//----------------------------------------------------------------------------
void
   fInfo( void )                    // Display local information
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       fParm
//
// Purpose-
//       Analyze a parameter.
//
//----------------------------------------------------------------------------
int                                 // TRUE if valid
   fParm(                           // Analyze a parameter
     const char*     argp)          // -> Argument
{
   return FALSE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       fShow
//
// Purpose-
//       Display local values.
//
//----------------------------------------------------------------------------
void
   fShow( void )                    // Display local values
{
}

#endif // FUNCTION_H_INCLUDED
