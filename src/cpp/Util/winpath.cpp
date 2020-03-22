//----------------------------------------------------------------------------
//
//       Copyright (c) 2015 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       winpath.cpp
//
// Purpose-
//       Change '/' to '\\','\\' in arguments
//
// Last change date-
//       2015/01/01
//
// Usage notes-
//       This is sometimes more useful than cygpath for converting unix file
//       name parameters.
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

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
   for(int i= 1; i<argc; i++)
   {
     if( i > 1 )
       putchar(' ');

     int   M = strlen(argv[i]);
     for(int j= 0; j<M; j++)
     {
       char C = argv[i][j];
       if( C == '/' )
       {
         C = '\\';
         putchar(C);
       }

       putchar(C);
     }
   }

   return 0;
}

