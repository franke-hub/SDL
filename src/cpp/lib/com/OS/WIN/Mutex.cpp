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
//       OS/WIN/Mutex.cpp
//
// Purpose-
//       Mutex object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include <com/define.h>
#include <com/Exception.h>
#include <com/Debug.h>
#include <com/Thread.h>
#include "com/Mutex.h"

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
// Subroutine-
//       Mutex::~Mutex
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Mutex::~Mutex( void )            // Destructor
{
   IFHCDM( printf("%8s= Mutex(%p)::~Mutex\n", "", this); )

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
//       Mutex::Mutex
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Mutex::Mutex( void )             // Constructor
:  object(NULL)
{
   IFHCDM( printf("%8s= Mutex(%p)::Mutex\n", "", this); )

   HANDLE handle= ::CreateMutex(    // Create the physical Mutex
                      NULL,         // (No security attribute)
                      FALSE,        // (No initial owner)
                      NULL);        // Unnamed Mutex
   if( handle == NULL)
     throwf("%4d %s CreateMutex", __LINE__, __FILE__);

   object= handle;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Mutex::release
//
// Purpose-
//       Release control of the Mutex object.
//
//----------------------------------------------------------------------------
void
   Mutex::release( void )           // Release the Mutex object
{
   HANDLE handle= object;
   ::ReleaseMutex(handle);

   IFHCDM( printf("%8s= Mutex(%p)::release() H(%p)\n", "", this, handle); )
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Mutex::reserve
//
// Purpose-
//       Reserve control of the Mutex object.
//
//----------------------------------------------------------------------------
void
   Mutex::reserve( void )           // Reserve the Mutex object
{
   HANDLE handle= object;

   for(;;)
   {
     int rc= WaitForSingleObject(handle, INFINITE);
     if( rc != WAIT_TIMEOUT )
       break;
   }

   IFHCDM( printf("%8s= Mutex(%p)::reserve H(%p)\n", "", this, handle); )
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Mutex::debugRelease
//
// Purpose-
//       Release control of the Mutex object.
//
//----------------------------------------------------------------------------
void
   Mutex::debugRelease(             // Reserve the Mutex object
     const char*       file,        // File name
     int               line)        // Line number
{
   debugf("%s %d: T(%p) Mutex(%p)::released\n",
          file, line, Thread::current(), this);

   release();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Mutex::debugReserve
//
// Purpose-
//       Reserve control of the Mutex object.
//
//----------------------------------------------------------------------------
void
   Mutex::debugReserve(             // Reserve the Mutex object
     const char*       file,        // File name
     int               line)        // Line number
{
   debugf("%s %d: T(%p) Mutex(%p)::blocking\n",
          file, line, Thread::current(), this);
   reserve();
   debugf("%s %d: T(%p) Mutex(%p)::reserved\n",
          file, line, Thread::current(), this);
}

