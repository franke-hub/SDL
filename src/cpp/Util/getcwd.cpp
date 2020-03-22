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
//       getcwd.cpp
//
// Purpose-
//       Get Current Working Directory.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>

#include <com/Software.h>

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
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   char                cwd[2048];   // Resultant area

   //-------------------------------------------------------------------------
   // Process GETCWD command
   //-------------------------------------------------------------------------
   printf("%s\n", Software::getCwd(cwd, sizeof(cwd)));

   return 0;
}

