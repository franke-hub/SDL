//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Main.cpp
//
// Purpose-
//       Sample C++ program.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include "logger.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "MAIN    " // Source file

//----------------------------------------------------------------------------
// PROTOTYPES
//----------------------------------------------------------------------------
extern int malloc(int argc, char* argv[]);
extern int sample(int argc, char* argv[]);

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
   int                 errorCount= 0; // Error counter

   openlog("Generic", LOG_CONS | LOG_PID, LOG_USER);

   errorCount += sample(argc, argv);
   errorCount += malloc(argc, argv);

   if( errorCount == 0 )
     syslog(LOG_INFO, "OK\n");
   else
     syslog(LOG_INFO, "NG, %d errors\n", errorCount);

   closelog();

   return 0;
}

