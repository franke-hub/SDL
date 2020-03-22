//----------------------------------------------------------------------------
//
//       Copyright (c) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ThreadLogger.cpp
//
// Purpose-
//       Implement ThreadLogger object methods
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <com/Clock.h>
#include <com/Thread.h>

#include "com/ThreadLogger.h"       // Includes Debug.h

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Thread*         systemThread= NULL; // THE system Thread

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLogger::~ThreadLogger
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   ThreadLogger::~ThreadLogger( void ) // Destructor
{
   logf("ThreadLogger(%p)::~ThreadLogger()\n", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLogger::ThreadLogger
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   ThreadLogger::ThreadLogger(      // Constructor
     const char*       name)        // The file name
:  Logger(name)
{
   systemThread= Thread::current(); // The current thread is SystemThread
   logf("ThreadLogger(%p)::ThreadLogger()\n", this);
   if( sizeof(void*) == 8 )
     logf("SystemThread(0x%.16lx)\n", (unsigned long)(uintptr_t)systemThread);
   else
     logf("SystemThread(0x%.8lx)\n", (unsigned long)(uintptr_t)systemThread);
}

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLogger::logf
//
// Purpose-
//       Write message to log.
//
//----------------------------------------------------------------------------
void
   ThreadLogger::logf(              // Write log message
     const char*       fmt,         // PRINTF format descriptor
                       ...)         // PRINTF argruments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vlogf(fmt, argptr);              // Write to log
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLogger::vlogf
//
// Purpose-
//       Write message to log.
//
//----------------------------------------------------------------------------
void
   ThreadLogger::vlogf(             // Write log message
     const char*       fmt,         // PRINTF format descriptor
     va_list           argptr)      // PRINTF arguments
{
   char                buffer[32];  // Thread name buffer
   NamedThread*        thread;      // Current thread
   const char*         threadName;  // Current thread name

   Thread* current= Thread::current();
   thread= dynamic_cast<NamedThread*>(current);
   if( thread == NULL )
   {
     if( current == systemThread )
       threadName= "SystemThread";
     else
     {
       if( sizeof(void*) == 8 )
         sprintf(buffer, "**%.16lx**", (unsigned long)(uintptr_t)current);
       else
         sprintf(buffer, "**%.8lx**", (unsigned long)(uintptr_t)current);
       threadName= buffer;
     }
   }
   else
     threadName= thread->getName();

   int cc= obtain();
   tracef("%14.3f <%s> ", Clock::current(), threadName);
   vtracef(fmt, argptr);
   if( cc == 0 )
     release();
}

