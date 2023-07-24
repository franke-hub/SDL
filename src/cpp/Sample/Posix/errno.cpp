//----------------------------------------------------------------------------
//
//       Copyright (c) 2013 Frank Eskesen.
//
//       This file is free content, distributed under creative commons CC0,
//       explicitly released into the Public Domain.
//       (See accompanying html file LICENSE.ZERO or the original contained
//       within https://creativecommons.org/publicdomain/zero/1.0/legalcode)
//
//----------------------------------------------------------------------------
//
// Title-
//       errno.cpp
//
// Purpose-
//       Test errno (by examining the object code.)
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#include <errno.h>
#include <stdio.h>

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
   main(int, char**)                // Mainline code
//   int             argc,          // Argument count (Unused)
//   char*           argv[])        // Argument array (Unused)
{
   errno= 27;
   int copy= errno;
   printf("errno(%d)\n", copy);
   printf("errno(%d)\n", errno);
   return 0;
}

