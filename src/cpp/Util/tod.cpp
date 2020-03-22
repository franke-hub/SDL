//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       tod.cpp
//
// Purpose-
//       Get time of day.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Function-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   struct tm           t;           // Time information
   time_t              tod;         // Time of day

   tod= time(NULL);                 // Get current time of day
   t= *localtime(&tod);

   printf("Current date: %.2d/%.2d/%.4d\n",
          t.tm_mon+1, t.tm_mday, t.tm_year+1900);

   printf("Current time: %.2d:%.2d:%.2d\n",
          t.tm_hour, t.tm_min, t.tm_sec);

   return 0;
}

