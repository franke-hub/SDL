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
//       OS/BSD/Semaphore.cpp
//
// Purpose-
//       Semaphore object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <errno.h>
#include <malloc.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>

#include <com/Clock.h>
#include <com/Debug.h>
#include <com/define.h>
#include <com/Exception.h>
#include <com/Thread.h>
#include <com/Unconditional.h>

#include "com/Semaphore.h"

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
//       Describe the hidden Semaphore object.
//
//----------------------------------------------------------------------------
struct Object {                     // The hidden Semaphore object
sem_t                  semaphore;   // The hidden Semaphore object
}; // struct Object

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
   IFHCDM( debugf("%8s= Semaphore(%p)::~Semaphore\n", "", this); )

   Object* O= (Object*)object;
   if( O != NULL)
   {
     sem_destroy(&O->semaphore);
     object= NULL;
     free(O);
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
   IFHCDM( debugf("%8s= Semaphore(%p)::Semaphore\n", "", this); )

   Object* O= (Object*)must_malloc(sizeof(Object));
   memset(O, 0, sizeof(Object));

   object= O;                       // Save the object pointer
   sem_init(&O->semaphore, 0, count); // Initialize
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Semaphore::post
//
// Purpose-
//       Post the Semaphore object.
//
//----------------------------------------------------------------------------
void
   Semaphore::post( void )          // Post the Semaphore (Make available)
{
   Object* O= (Object*)object;
   sem_post(&O->semaphore);

   IFHCDM( debugf("%8s= Semaphore(%p)::post()\n", "", this); )
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Semaphore::wait
//
// Purpose-
//       Wait for the Semaphore object.
//
//----------------------------------------------------------------------------
void
   Semaphore::wait( void )          // Wait for Semaphore object
{
   Object* O= (Object*)object;

   for(;;)
   {
     int rc= sem_wait(&O->semaphore); // Obtain control
     if( rc == 0 )
       break;

     if( errno != EINTR )
       throwf("%4d %s Unexpected errno(%d)", __LINE__, __FILE__, errno);
   }

   IFHCDM( debugf("%8s= Semaphore(%p)::wait\n", "", this); )
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
   Object*             O= (Object*)object;
   int                 rc;          // Return code

   // Set expiration time
   struct timespec timeout;         // Absolute timeout

   if( delay <= 0.0 )               // If trywait
     rc= sem_trywait(&O->semaphore); // Attempt to obtain control
   else
   {
     delay += Clock::current();     // Expiration time (absolute seconds)

     timeout.tv_sec= int(delay);    // Expiration abolute seconds
     delay= delay - double(int(delay)); // Fractional remainder
     delay *= 1000000000.0;         // (in nanoseconds)
     timeout.tv_nsec= int(delay);   // Remainder (in nanoseconds)

     // Wait (with timeout)
     rc= sem_timedwait(&O->semaphore, &timeout);
   }

   IFHCDM( debugf("%8s= %d= Semaphore(%p)::wait(%.3f)\n", "", rc, this, delay); )

   return rc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Semaphore::debugPost
//
// Purpose-
//       Post the Semaphore object.
//
//----------------------------------------------------------------------------
void
   Semaphore::debugPost(            // Post the Semaphore (Make available)
     const char*       file,        // File name
     int               line)        // Line number
{
   debugf("%s %d: T(%p) Semaphore(%p)::posted\n",
          file, line, Thread::current(), this);
   post();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Semaphore::debugWait
//
// Purpose-
//       Wait for Semaphore object.
//
//----------------------------------------------------------------------------
void
   Semaphore::debugWait(            // Wait for Semaphore object
     const char*       file,        // File name
     int               line)        // Line number
{
   debugf("%s %d: T(%p) Semaphore(%p)::blocking\n",
          file, line, Thread::current(), this);
   wait();
   debugf("%s %d: T(%p) Semaphore(%p)::accessed\n",
          file, line, Thread::current(), this);
}

