//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_str.cpp
//
// Purpose-
//       Test istring functions.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include "com/istring.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "Test_str" // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       tester
//
// Purpose-
//       Hard core debug mode testing.
//
//----------------------------------------------------------------------------
static void
   tester(                          // Test character
     char            c)             // This character
{
   #if 1
     printf("{%c,%d}= toupper({%c,%d})\n", toupper(c), toupper(c), c, c);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testLE
//
// Purpose-
//       stricmp test.
//
//----------------------------------------------------------------------------
static int                          // Error count
   testLE(                          // Test stricmp
     const char*     p1,            // Parameter[1]
     const char*     p2)            // Parameter[2]
{
   int               cc;

   cc= stricmp(p1,p2);
   if( cc <= 0 )
     return 0;

   debugf("%d= stricmp(%s,%s)\n", cc, p1, p2);
   return 1;
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
extern int
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   int               errorCount= 0;
   int               rc;

   //-------------------------------------------------------------------------
   // Information
   //-------------------------------------------------------------------------
   tester('a');
   tester('A');
   tester('_');

   //-------------------------------------------------------------------------
   // Built-in tests
   //-------------------------------------------------------------------------
   errorCount += testLE("alpha", "alpha");
   errorCount += testLE("alpha", "ALPHA");
   errorCount += testLE("ALPHA", "alpha");
   errorCount += testLE("alpha", "beta ");
   errorCount += testLE("alpha", "gamma");
   errorCount += testLE("beta ", "beta ");
   errorCount += testLE("beta ", "gamma");
   errorCount += testLE("gamma", "gamma");

   //-------------------------------------------------------------------------
   // Parameter test
   //-------------------------------------------------------------------------
   if( argc >= 3 )
   {
     rc= stricmp(argv[1],argv[2]);
     printf("%d= stricmp(%s,%s)\n", rc, argv[1], argv[2]);
   }

   //-------------------------------------------------------------------------
   // Testing complete
   //-------------------------------------------------------------------------
   debugf("%s complete, ", __SOURCE__);
   if( errorCount == 0 )
     debugf("NO ");
   else
     debugf("%d ", errorCount);
   debugf("Error%s\n", errorCount == 1 ? "" : "s");

   return errorCount;
}

