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
//       Format.cpp
//
// Purpose-
//       Format the stock data.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/Reader.h>
#include <com/Writer.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "FORMAT  " // Source file, for debugging

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
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   Reader            reader("composite.inp");
   Writer            writer("composite.out");

   int               c;
   int               rc;

   //-------------------------------------------------------------------------
   // Remove extra line delimiters
   //-------------------------------------------------------------------------
   for(;;)
   {
     c= reader.get();

     if( c == '\n' || c == '\r' )
     {
       while( c == '\n' || c == '\r' )
         c= reader.get();

       writer.put('\r');
       writer.put('\n');
     }

     if( c == EOF )
       break;

     writer.put(c);
   }

   return 0;
}

