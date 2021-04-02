//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2021 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_bug.cpp
//
// Purpose-
//       Test debugging methods.
//
// Last change date-
//       2021/04/02
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include "com/Debug.h"
#include "com/Logger.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TEST_BUG" // Source file, for debugging

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Debug         debug;         // Debug object

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
//   int             argc,          // Argument count
//   char*           argv[])        // Argument array
{
   char              buff[128];
   size_t            i;

   for(i=0; i<sizeof(buff); i++)
     buff[i]= 0x80 + i;

   debugSetStandardMode();
   debugf("Standard mode:\n");
   tracef("This appears only in the %s file\n", "TRACE");
   debugf("This appears in %s and %s\n", "TRACE", "STDOUT");
   errorf("This appears in %s and %s\n", "TRACE", "STDERR");

   debugSetIntensiveMode();
   debugf("Intensive mode:\n");
   tracef("This appears only in the %s file\n", "TRACE");
   debugf("This appears in %s and %s\n", "TRACE", "STDOUT");
   errorf("This appears in %s and %s\n", "TRACE", "STDERR");

   tracef("Dump(%p,%x)\n", &debug, (int)sizeof(debug));
   dump(&debug, sizeof(debug));

   debugf("\n");
   debugf("Snap(%p,%x)\n", &debug, (int)sizeof(debug));
   snap(&debug, sizeof(debug));

   #if 0
     debugf("\n");
     debugf("Snap(%p,%x)\n", buff, (int)sizeof(buff));
     snap(buff, sizeof(buff));

     debugf("\n");
     debugf("Snap(%p,%x)\n", buff+3, (int)sizeof(buff)-6);
     snap(buff+3, sizeof(buff)-6);

     debugf("\n");
     debugf("Snap(%p,%x)\n", buff+5, 5);
     snap(buff+5, 5);

     debugf("\n");
     debugf("Snap(%p,%x)\n", buff+11, 5);
     snap(buff+11, 5);

     debugf("\n");
     debugf("Snap(%p,%x)\n", buff+12, 5);
     snap(buff+12, 5);
   #endif

   debugf("\n");
   debugf("Snap(%p,%x)\n", buff+14, 4);
   snap(buff+14, 4);

   debugf("\n");
   debugf("Snap(%p,%zx)\n", buff, sizeof(buff));
   memset(buff, 0, sizeof(buff));
   snap(buff, sizeof(buff));

   Logger::log("Initial mode:\n");
   Logger::get()->logf("Message %s\n", "Logger::get->logf");
   Logger::log("Message %s\n", "Logger::log");
   traceh("Message %s\n", "traceh");

   Logger::log("Intensive mode:\n");
   Logger::get()->setMode(Logger::ModeIntensive);
   Logger::get()->logf("Message %s\n", "Logger::get->logf");
   Logger::log("Message %s\n", "Logger::log");
   traceh("Message %s\n", "traceh");

   Logger::log("Standard mode:\n");
   Logger::get()->setMode(Logger::ModeStandard);
   Logger::get()->logf("Message %s\n", "Logger::get->logf");
   Logger::log("Message %s\n", "Logger::log");
   traceh("Message %s\n", "traceh");

   Logger::log("Ignore mode:\n");
   Logger::get()->setMode(Logger::ModeIgnore);
   Logger::get()->logf("Message %s\n", "Logger::get->logf");
   Logger::log("Message %s\n", "Logger::log");
   traceh("Message %s\n", "traceh");

   return 0;                        // Normal completion
}

