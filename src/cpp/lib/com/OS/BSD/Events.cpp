//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/BSD/Events.cpp
//
// Purpose-
//       Events object methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
sem_t                  mutex;       // The physical semaphore
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
     sem_destroy(&O->mutex);
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

   int rc= sem_init(&O->mutex, 0, 0); // Initialize (LOCKED)
   if( rc < 0 )
     throwf("%4d %s %d= sem_init", __LINE__, __FILE__, rc);

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
     int rc= sem_wait(&O->mutex);   // Obtain control
     if( rc == 0 )
       break;

     if( errno != EINTR )
       throwf("%4d %s Unexpected error(%d)", __LINE__, __FILE__, errno);
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
   sem_post(&O->mutex);             // Release the Mutex

   #ifdef HCDM
     debugf("%8s= Events(%p)::post()\n", "", this);
   #endif
}

