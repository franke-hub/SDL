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
//       sigmoid.h
//
// Purpose-
//       Define the sigmoid function: K * sin(R * x)
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
static double        lower= -10.0;  // Evaluate lower
static double        upper= +10.0;  // Evaluate upper
static double        steps= 128.0;  // Evaluation steps

//----------------------------------------------------------------------------
//
// Subroutine-
//       f
//
// Purpose-
//       The function to be Evaluated
//
//----------------------------------------------------------------------------
static inline double                // Resultant (y)
   f(                               // The function
     double          x)             // Argument  (x)
{
   return 1.0 / (1.0 + exp(-x));
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
static inline const char*           // The function name
   fName( void )                    // The function's name
{
   return "sigmoid(x)= 1.0 / (1.0 + exp(-x))";
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
static inline void
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
static inline int                   // TRUE if valid
   fParm(                           // Analyze a parameter
     const char*     argp)          // -> Argument
{
   (void)argp;                      // (Parameter ignored)
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
static inline void
   fShow( void )                    // Display local values
{
}

#endif // FUNCTION_H_INCLUDED
