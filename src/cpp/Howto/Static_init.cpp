//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Static_init.cpp
//
// Purpose-
//       Sample program: Anonymous static initializer / terminator
//
// Last change date-
//       2020/12/16
//
//----------------------------------------------------------------------------
#include <stdio.h>                  // For printf

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
unsigned char          buffer[256]; // Working buffer

namespace {                         // Anonymous namespace
struct INIT {                       // Initializer/terminator
   INIT( void )                     // Initializer
{
   printf("%4d INIT::INIT\n", __LINE__);

   for(int i= 0; i<(int)sizeof(buffer); i++)
     buffer[i]= i;
}

   ~INIT( void )                    // Terminator
{
   printf("%4d INIT::~INIT\n", __LINE__);
   test();
}

void test( void )                   // Tester
{
   printf("%4d INIT::test\n", __LINE__);

   for(int i= 0; i<(int)sizeof(buffer); i++) {
     if( buffer[i] != i )
       printf("%4d Someone (not saying who) needs to RTFM\n", __LINE__);
   }
}
}  static_initializer;
}  // Anonymous namespace

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
   printf("main...\n");
   static_initializer.test();
   printf("...main\n");

   return 0;
}
