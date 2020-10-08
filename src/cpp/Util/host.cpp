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
//       host.cpp
//
// Purpose-
//       Get host information.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>

#include <com/Socket.h>

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
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{
   Socket::Addr        addr;        // Working address

   //-------------------------------------------------------------------------
   // Get host name
   //-------------------------------------------------------------------------
   printf("Name(%s)\n", Socket::getName());

   //-------------------------------------------------------------------------
   // Get host address
   //-------------------------------------------------------------------------
   addr= Socket::getAddr();
   if( addr >= uint64_t(0x0000000100000000) )
     printf("Addr(%d.%d.%d.%d::%d.%d.%d.%d)\n"
            , (int)((addr >> 56 ) & 0x00ff)
            , (int)((addr >> 48 ) & 0x00ff)
            , (int)((addr >> 40 ) & 0x00ff)
            , (int)((addr >> 32 ) & 0x00ff)
            , (int)((addr >> 24 ) & 0x00ff)
            , (int)((addr >> 16 ) & 0x00ff)
            , (int)((addr >>  8 ) & 0x00ff)
            , (int)((addr >>  0 ) & 0x00ff)
            );
   else
     printf("Addr(%d.%d.%d.%d)\n"
            , (int)((addr >> 24 ) & 0x00ff)
            , (int)((addr >> 16 ) & 0x00ff)
            , (int)((addr >>  8 ) & 0x00ff)
            , (int)((addr >>  0 ) & 0x00ff)
            );

   return 0;
}

