//----------------------------------------------------------------------------
//
//       Copyright (c) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/WIN/Semaphore.cpp
//
// Purpose-
//       Semaphore object methods.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include <com/define.h>
#include <com/Exception.h>
#include <com/Debug.h>
#include <com/Thread.h>
#include "com/Semaphore.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SEM_VALUE_MAX
#define SEM_VALUE_MAX 32767         // Using _POSIX_SEM_VALUE_MAX
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Subroutine-
//       Semaphore::~Semaphore
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Semaphore::~Semaphore( void )    // Destructor
{
   IFHCDM( printf("%8s= Semaphore(%p)::~Semaphore\n", "", this); )

   HANDLE handle= object;
   if (handle != NULL)
   {
     object= NULL;
     ::CloseHandle(handle);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Semaphore::Semaphore
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Semaphore::Semaphore(            // Constructor
     int               count)       // Initial share count
:  object(NULL)
{
   IFHCDM( printf("%8s= Semaphore(%p)::Semaphore(%d)\n", "", this, count); )

   HANDLE handle= ::CreateSemaphore(// Create the physical Semaphore
                      NULL,         // (No security attribute)
                      count,        // Share count
                      SEM_VALUE_MAX,// Maximum share count
                      NULL);        // Unnamed Semaphore
   if( handle == NULL)
     throwf("%4d %s CreateSemaphore", __LINE__, __FILE__);

   object= handle;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Semaphore::post
//
// Purpose-
//       Signal Semaphore object available.
//
//----------------------------------------------------------------------------
void
   Semaphore::post( void )          // Post the Semaphore available
{
   HANDLE handle= object;
   ::ReleaseSemaphore(handle, 1, NULL);

   IFHCDM( printf("%8s= Semaphore(%p)::post H(%p)\n", "", this, handle); )
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Semaphore::wait
//
// Purpose-
//       Wait for the Semaphore object to become available.
//
//----------------------------------------------------------------------------
void
   Semaphore::wait( void )          // Wait for the Semaphore availablity
{
   HANDLE handle= object;

   for(;;)
   {
     int rc= WaitForSingleObject(handle, INFINITE);
     if( rc != WAIT_TIMEOUT )
       break;
   }

   IFHCDM( printf("%8s= Semaphore(%p)::wait() H(%p)\n", "", this, handle); )
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Semaphore::wait(double)
//
// Purpose-
//       Attempt wait for the Semaphore object.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 iff successful)
   Semaphore::wait(                 // Attempt wait for Semaphore object
     double            delay)       // Timeout delay
{
   HANDLE handle= object;

   // Set expiration delay
   if( delay < 0.0 )
     delay= 0.0;

   delay *= 1000.0;                 // Delay (in milliseconds)
   delay += 0.5;                    // Round upwards

   // Wait (with timeout)
   int rc= WaitForSingleObject(handle, int(delay));
   if( rc == WAIT_ABANDONED )
     rc= WAIT_OBJECT_0;

   IFHCDM( debugf("%8s %d= Semaphore(%p)::wait(%f) H(%p)\n", "",
                  rc, this, delay, handle); )

   return rc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Semaphore::debugPost
//
// Purpose-
//       Signal Semaphore object available
//
//----------------------------------------------------------------------------
void
   Semaphore::debugPost(            // Post the Semaphore available
     const char*       file,        // File name
     int               line)        // Line number
{
   debugf("%s %d: T(%p) Semaphore(%p)::post\n",
          file, line, Thread::current(), this);

   post();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Semaphore::debugWait
//
// Purpose-
//       Wait for the Semaphore object.
//
//----------------------------------------------------------------------------
void
   Semaphore::debugWait(            // Wait for Semaphore availability
     const char*       file,        // File name
     int               line)        // Line number
{
   debugf("%s %d: T(%p) Semaphore(%p)::blocking\n",
          file, line, Thread::current(), this);
   wait();
   debugf("%s %d: T(%p) Semaphore(%p)::wait\n",
          file, line, Thread::current(), this);
}

