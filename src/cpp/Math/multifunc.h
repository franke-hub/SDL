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
//       multifunc.h
//
// Purpose-
//       Define the function: (multiple functions defined)
//
// Last change date-
//       2007/01/01
//
// Notes:
//       F(x) = R + K * sin(x)
//
//       f0(x)= x0prime
//       f1(x)= y0prime
//                           .
//                      .    F(x+h)
//                 .         |
//            .              |
//       F(x)                |
//    ---|<------- h ------->|
//    |  | \.
//    |  |  \.
//    |  |   \.
//    |  |    \.
//    |  |     \.
//    |  |      \.
//    |  |       \y
//    |  |        \0
//    |  |         \p
//    R  |          \r
//    |  |           \i
//    |  |            \m
//    |  |             \e
//    |  |              \.
//    |  |               \.
//    |  |                \.
//    |  |                 \.
//    |  |                  \.
//    ---|-------------------.------
//       x0                  x0prime
//
//       F(x+h) - F(x)      x0prime - x0                      F(x+h) - F(x)
//       -------------   =  ------------ ; x0prime= x0 + F(x) -------------
//             h                F(x)                                h
//
//
//       y0prime = sqrt( (x0prime-x0)^2 + F(x)^2 )
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
static double        upper= 2.0 * (M_PI) + .1; // Evaluate upper
static double        steps= 128.0;  // Evaluation delta

//----------------------------------------------------------------------------
// Locals
//----------------------------------------------------------------------------
static int           sw_func= 0;    // Function identifier

static double        epsilon= 1.0E-6; // Small number
static double        K= 0.1;        // Constant
static double        R= 0.1;        // Constant

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
     double          x0)            // Argument  (x)
{
   double            y0, yh;
   double            x0prime;
   double            y0prime;

   y0= K * sin(x0);
   yh= K * sin(x0 + epsilon);

   x0prime= x0 + (R + y0) * ( (yh - y0) / epsilon );
   y0prime= sqrt((x0prime-x0)*(x0prime-x0) + (y0+R)*(y0+R));

   switch(sw_func)
   {
     case 0:
       return x0prime;

     case 1:
       return y0prime;

     case 999:
       return K * x0;

     default:
       fprintf(stderr, "Invalid function(%d)\n", sw_func);
       exit(EXIT_FAILURE);
   }

   return 0.0;
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
   fName( void )                    // Get function's name
{
   switch(sw_func)
   {
     case 0:
       return "x0prime of R + K * sin(x0)";

     case 1:
       return "y0prime of R + K * sin(x0)";

     case 999:
       return "K * x";

     default:
       fprintf(stderr, "Invalid function(%d)\n", sw_func);
       exit(EXIT_FAILURE);
   }

   return "Undefined";
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

        if ( swname("-func:", argp) )
     sw_func= swatol("-func:", argp);

   else if ( swname("-K:", argp) )
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
   printf("%12.0d = func\n", sw_func);
}

#endif // FUNCTION_H_INCLUDED
