//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Testmoid.cpp
//
// Purpose-
//       TEST Sigmoid function
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <math.h>
#include <stdio.h>

#include "Neuron.h"
#include "Fanin.h"

//----------------------------------------------------------------------------
//
// Subroutine-
//       sigmoid
//
// Purpose-
//       Compute sigmoid(x).
//
// Returns-
//       Output signal
//
//----------------------------------------------------------------------------
static double
   sigmoid(                         // Sigmoid function
     double            x)           // Argument
{
   double              y;           // Output value

   y= 1.0 / (1.0 + exp(-x));        // Compute the value
   return(y);
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
   main(int, char**)                // Sigmoid test program
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{
   double              x;

   printf("%ld= sizeof(Neuron)\n", (long)sizeof(Neuron));
   printf("%ld= sizeof(Fanin)\n", (long)sizeof(Fanin));


   for(x= (-20.0); x<=20.0; x += 1.0)
     printf("%10.6f= sigmoid(%6.2f)\n", sigmoid(x), x);

   return 0;
}

