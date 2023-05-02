//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Logger.cpp
//
// Purpose-
//       Logger implementation
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "com/Logger.h"

#if defined(_OS_WIN)
  #include <windows.h>
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Method-
//       Logger::~Logger
//
// Purpose-
//       Deconstructor.
//
//----------------------------------------------------------------------------
   Logger::~Logger( void )          // Deconstructor
{
   IFHCDM( fprintf(stderr, "%4d: Logger(%p)::~Logger()\n", __LINE__, this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       Logger::Logger
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Logger::Logger(                  // Constructor
     const char*       name)        // The debug file name, default "debug.out"
:  Debug(name)
{
   IFHCDM( fprintf(stderr, "%4d: Logger(%p)::Logger(%s)\n", __LINE__, this, name); )
}

//----------------------------------------------------------------------------
//
// Method-
//       Logger::init
//
// Function-
//       Activate the trace file.
//
// Implementation nodes-
//       Caller must hold Barrier latch.
//
//----------------------------------------------------------------------------
void
   Logger::init( void )             // Activate the trace file
{
   IFHCDM( fprintf(stderr, "Logger(%p)::init()\n", this); )

   if( handle == NULL )             // If still not active
   {
     if( isSTDIO(fileName) )
     {
       if( fileName[0] == '>' || fileName[0] == '1' )
         handle= stdout;
       else
         handle= stderr;
     }
     else
     {
       handle= fopen(fileName, "a+"); // Open the trace file (append mode)
       if( handle == NULL )         // If the open failed
       {
         fprintf(stderr, "DEBUG: Error: file(%s) open error\n", fileName);
         handle= stderr;
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Logger::log
//
// Purpose-
//       Write a message to the debugging log
//
//----------------------------------------------------------------------------
void
   Logger::log(                     // Write debugging log message
     const char*       format,      // PRINTF format message
                       ...)         // PRINTF arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, format);        // Initialize va_ functions
   Logger::get()->vlogf(format, argptr); // Write message to log
   va_end(argptr);                  // Close va_ functions
}

