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
//       main.cpp
//
// Purpose-
//       Wrapper to call tlc.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

extern "C" void
   tlc( void );                     // Threaded Language Compiler

//----------------------------------------------------------------------------
//
// Subroutine-
//       getSTDIN
//
// Purpose-
//       Read from stdin.
//
//----------------------------------------------------------------------------
extern "C" char*                    // Return address
   getSTDIN(                        // Mainline code
     char*           addr,          // Data address
     unsigned        size)          // Data length
{
   char*             C;
   unsigned          L;

   C= fgets(addr, size, stdin);
   if( C != NULL )
   {
     L= strlen(addr);
     if( L > 0 && addr[L-1] == '\n' )
       addr[L-1]= '\0';
   }

   return C;
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
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   tlc();
   return 0;
}

