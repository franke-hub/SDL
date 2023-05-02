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
//       Test_Sys.cpp
//
// Purpose-
//       Test system library functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TEST_SYS" // Source file, for debugging

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
     char*           argv[])        // Argument array
{
   char              chs[128];      // Character string
   char*             s;             // Pointer to string
   int               l;             // Generic length

   //-------------------------------------------------------------------------
   // strlen() function
   //-------------------------------------------------------------------------
   s= "This is a string";
   l= strlen(s);

   printf("%d= strlen(\"%s\") %s\n", l, s, l==16 ? "OK" : "NG");

   //-------------------------------------------------------------------------
   // sprintf() function
   //-------------------------------------------------------------------------
   sprintf(chs, "%s", "This is a string");

   l= strlen(chs);
   printf("%d= strlen(\"%s\") %s\n", l, chs, l==16 ? "OK" : "NG");

   sprintf(chs, "<%.4d> BG(%s) FG(%s), c('%c') %3d",
           0, "White", "Blue", 'a', 'a');
   l= strlen(chs);
   sprintf(chs, "<%.4d> BG(%s) FG(%s), c('%c') %3d",
           l, "White", "Blue", 'a', 'a');

   printf("%d= strlen(\"%s\") %s\n", l, chs, l==37 ? "OK" : "NG");


   return 0;                        // Normal completion
}

