//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/WIN/Events.cpp
//
// Purpose-
//       Events object methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <limits.h>
#include <windows.h>

#include <com/Exception.h>
#include <com/define.h>
#include <com/Debug.h>
#include <com/Unconditional.h>

#include "com/Events.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Struct-
//       Object
//
// Purpose-
//       Describe our hidden Events Object.
//
//----------------------------------------------------------------------------
struct Object {                     // The physical Events Object
HANDLE                 handle;      // The Semaphore handle
}; // struct Object

//----------------------------------------------------------------------------
//
// Subroutine-
//       Events::~Events
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Events::~Events( void )          // Destructor
{
   #ifdef HCDM
     debugf("%8s= Events(%p)::~Events\n", "", this);
   #endif

   Object* O= (Object*)object;      // Address the object, if it exists
   if( O != NULL )
   {
     CloseHandle(&O->handle);
     object= NULL;
     free(O);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Events::Events
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Events::Events( void )           // Constructor
:  object(NULL)
{
   #ifdef HCDM
     debugf("%8s= Events(%p)::Events\n", "", this);
   #endif

   Object* O= (Object*)must_malloc(sizeof(Object)); // Allocate a new Object
   memset(O, 0, sizeof(*O));

   O->handle= CreateSemaphore(NULL, 0, INT_MAX, NULL); // Initialize (LOCKED)
   if( O->handle == NULL )
     throwf("%4d %s NULL= CreateSemaphore()", __LINE__, __FILE__);

   object= O;                       // Save the object pointer
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Events::wait
//
// Purpose-
//       Wait for control of the Events object.
//
//----------------------------------------------------------------------------
void
   Events::wait( void )             // Obtain control of the Events object
{
   #ifdef HCDM
     debugf("%8s= Events(%p)::wait()...\n", "", this);
   #endif

   Object* O= (Object*)object;      // Address the object, if it exists
   for(;;)
   {
     int rc= WaitForSingleObject(&O->handle, 0); // Wait for object
     if( rc == 0 )
       break;

     if( rc != WAIT_TIMEOUT )
       throwf("%4d %s Unexpected error(%d)\n", __LINE__, __FILE__, rc);
   }

   #ifdef HCDM
     debugf("...Events(%p)::wait()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Events::post
//
// Purpose-
//       Post the Events.
//
//----------------------------------------------------------------------------
void
   Events::post( void )             // Post the Events
{
   Object* O= (Object*)object;      // Address the object, if it exists
   ReleaseSemaphore(&O->handle, 1, NULL); // Post the Semaphore

   #ifdef HCDM
     debugf("%8s= Events(%p)::post()\n", "", this);
   #endif
}

