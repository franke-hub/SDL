//----------------------------------------------------------------------------
//
//       Copyright (c) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestFile.cpp
//
// Purpose-
//       Test Fileman.h (parts untested by ~/src/cpp/Fileman)
//
// Last change date-
//       2021/01/04
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <exception>

#include <pub/Debug.h>
#include "pub/Fileman.h"

using namespace _PUB_NAMESPACE::debugging;

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_name
//
// Purpose-
//       Test pub::fileman::Name
//
//----------------------------------------------------------------------------
static int                          // Error count
   test_name(                       // Test pub::fileman::Name
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 error_count= 0; // Number of errors encountered

// printf("%4d test_name\n", __LINE__);
   using namespace pub::fileman;

   // For argv[0], extract file name
   printf("'%s'= Name::get_file_name(%s)\n"
         , Name::get_file_name(argv[0]).c_str(), argv[0]);

   // Resolve each argument
   for(int argx= 1; argx < argc; argx++) {
     char* C= argv[argx];
     pub::fileman::Name name(C);
     std::string error= name.resolve();
     if( error == "" )              // If no error
       printf("OK: '%s'= resolve(%s)\n", name.name.c_str(), C);
     else {
       error_count++;
       printf("NG: '%s'= resolve(%s)\n", error.c_str(), C);
     }
   }

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
extern int
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 result= 0;

   try {
     result= test_name(argc, argv);
   } catch(const char* x) {
     debugf("Exception const char*(%s)\n", x);
     result= 2;
   } catch(std::exception& x) {
     debugf("Exception exception(%s)\n", x.what());
     result= 2;
   } catch(...) {
     debugf("Exception ...\n");
     result= 2;
   }

   printf("Result(%d)\n", result);

   return result;
}
