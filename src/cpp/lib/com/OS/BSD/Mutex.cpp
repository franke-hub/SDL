//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/BSD/Mutex.cpp
//
// Purpose-
//       Mutex object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <errno.h>
#include <malloc.h>
#include <semaphore.h>
#include <string.h>

#include <com/define.h>
#include <com/Exception.h>
#include <com/Debug.h>
#include <com/Thread.h>
#include <com/Unconditional.h>

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
// Struct-
//       Object
//
// Purpose-
//       Describe the hidden Mutex object.
//
//----------------------------------------------------------------------------
struct Object {                     // The hidden Mutex object
sem_t                  mutex;       // The hidden Mutex object
}; // struct Object

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
   IFHCDM( debugf("%8s= Mutex(%p)::~Mutex\n", "", this); )

   Object* O= (Object*)object;
   if( O != NULL)
   {
     sem_destroy(&O->mutex);
     object= NULL;
     free(O);
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
   IFHCDM( debugf("%8s= Mutex(%p)::Mutex\n", "", this); )

   Object* O= (Object*)must_malloc(sizeof(Object));
   memset(O, 0, sizeof(Object));

   object= O;                       // Save the object pointer
   sem_init(&O->mutex, 0, 1);       // Initialize (UNLOCKED)
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
   Object* O= (Object*)object;
   sem_post(&O->mutex);

   IFHCDM( debugf("%8s= Mutex(%p)::release()\n", "", this); )
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
   Object* O= (Object*)object;

   for(;;)
   {
     int rc= sem_wait(&O->mutex);   // Obtain control
     if( rc >= 0 )
       break;

     if( errno != EINTR )
       throwf("%4d %s Unexpected error(%d)", __LINE__, __FILE__, errno);
   }

   IFHCDM( debugf("%8s= Mutex(%p)::reserve\n", "", this); )
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
   Mutex::debugRelease(             // Release the Mutex object
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

