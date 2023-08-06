//----------------------------------------------------------------------------
//
//       Copyright (C) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Clean.cpp
//
// Purpose-
//       Some Quick and Dirty tests.
//
// Last change date-
//       2023/08/01
//
//----------------------------------------------------------------------------
#include <atomic>                   // We include everything and then some
#include <chrono>
#include <functional>
#include <iostream>
#include <list>
#include <new>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include <assert.h>
#include <endian.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>

#include <pub/TEST.H>
#include <pub/Debug.h>
#include <pub/Thread.h>
#include <pub/utility.h>

#include "Clean.h"

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*   zero= "\0";    // Expect ['\0','\0']
static const char*   junk= "junk";  // Expected to follow zero

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_case
//
// Purpose-
//       Cut and paste sample test
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors encountered
   test_case( void )                // Sample testcase
{
   int error_count= 0;              // Number of errors encountered

   printf("\ntest_case\n");

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_dirty
//
// Purpose-
//       The proverbial Quick and Dirty test.
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors encountered
   test_dirty( void )               // Today's Quick and Dirty test
{
   int error_count= 0;              // Number of errors encountered

   printf("\ntest_dirty\n");

   // Test: Does "\0" contain ['\0', '\0]? YES
   error_count += VERIFY( zero[0] == '\0' && zero[1] == '\0' );
   error_count += VERIFY( junk == zero + 2);

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_htons
//
// Purpose-
//       Test arpa/inet.h: htons, ntohs
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors encountered
   test_htons( void )               // Test arpa/inet.h: htohs, ntohs
{
   int error_count= 0;              // Number of errors encountered

   printf("\ntest_htons\n");
   unsigned char buffer[]= { 0x01, 0x02 };
   uint16_t* u16= (uint16_t*)buffer;
   printf("%.4x= htons(0102), %.4x= ntohs(0102)\n", htons(*u16), ntohs(*u16));
   printf("%.4x= htons(0203), %.4x= ntohs(0203)\n"
         , htons(0x0203), ntohs(0x0203));

   *u16= htons(0x0102);
   error_count += VERIFY( ntohs(*u16) == 0x0102 );
   error_count += VERIFY( buffer[0] ==  0x01 );
   error_count += VERIFY( buffer[1] ==  0x02 );
   printf("0x%.4x= *u16\n", *u16);

   return error_count;
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
   main(int, char**)                // Mainline code
//   int             argc,          // Argument count (Unused)
//   char*           argv[])        // Argument array (Unused)
{
   unsigned error_count= 0;

   try {
     printf("Dirty.cpp\n");

     // Run the tests
//   error_count += test_case();
     error_count += test_htons();

     error_count += test_dirty();

   } catch(std::exception& X) {
     error_count++;
     printf("%4d std::exception(%s)\n", __LINE__, X.what());
   } catch(...) {
     error_count++;
     printf("%4d catch(...)\n", __LINE__);
   }

   printf("\n");
   if( error_count == 0 )
     printf("NO errors detected\n");
   else if( error_count == 1 )
     printf("1 error detected\n");
   else {
     printf("%d errors detected\n", error_count);
     error_count= 1;
   }

   return error_count;
}
