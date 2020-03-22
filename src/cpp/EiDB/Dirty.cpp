//----------------------------------------------------------------------------
//
//       Copyright (C) 2003 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Dirty.cpp
//
// Purpose-
//       Quick and dirty test(s).
//
// Last change date-
//       2003/06/22
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <Random.h>
#include <Reader.h>

   double              peakCount, peakScale;

//----------------------------------------------------------------------------
//
// Subroutine-
//       f
//
// Purpose-
//       function
//
//----------------------------------------------------------------------------
double
   f(double count)
{
   double              result;

   result= pow(peakScale, (count - peakCount));
   result= pow(peakScale, (count - peakCount)/peakCount);

   return result;
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
int                                 // Return coe
   main(                            // Mainline code
     int               argc,        // Argument count
     const char*       argv[])      // Argument array
{
   double              count;

   peakCount= 100.0;
   peakScale= 100.0;
   for(count= 0; count<peakCount+1; count++)
   {
     printf("%12g= f(%g)\n", f(count), count);
   }

   return 0;
}

