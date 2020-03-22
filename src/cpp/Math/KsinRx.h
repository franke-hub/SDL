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
//       KsinRx.h
//
// Purpose-
//       Define the function: K * sin(R * x)
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
static double        upper= 2.0 * (M_PI); // Evaluate upper
static double        steps= 128.0;  // Evaluation steps

//----------------------------------------------------------------------------
// Locals
//----------------------------------------------------------------------------
static double        K= 1.0;        // Constant
static double        R= 1.0;        // Constant

//----------------------------------------------------------------------------
//
// Subroutine-
//       f
//
// Purpose-
//       The function to be evaluated
//
//----------------------------------------------------------------------------
inline double                       // Resultant (y)
   f(                               // The function
     double          x)             // Argument  (x)
{
   return K * sin(R*x);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       fName
//
// Purpose-
//       The name of the function to be evaluated
//
//----------------------------------------------------------------------------
const char*                         // The function name
   fName( void )                    // The function's name
{
   return "K * sin(R*x)";
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
   fprintf(stderr, "\n");
   fprintf(stderr, " -func:n       Function identifier\n");
   fprintf(stderr, " -K:value      Constant K\n");
   fprintf(stderr, " -R:value      Constant R\n");
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
   int               result= TRUE;  // Resultant

        if ( swname("-K:", argp) )
     K=  swatod("-K:", argp);

   else if ( swname("-R:", argp) )
     R=  swatod("-R:", argp);

   else
     result= FALSE;

   return result;
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
   printf("\n");
   printf("%12.3e = K\n", K);
   printf("%12.3e = R\n", R);
}

#endif // FUNCTION_H_INCLUDED
