//----------------------------------------------------------------------------
//
//       Copyright (C) 2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       mutex.cpp
//
// Purpose-
//       Implement mutex.h
//
// Last change date-
//       2026/03/23
//
// Implementation notes-
//       debugf (in Debug.cpp) formats and writes to stdout and "debug.out"
//       errorf (in Debug.cpp) formats and writes to stderr and "debug.out"
//       throwf (in Debug.cpp) formats and throws std::runtime_error
//
//----------------------------------------------------------------------------
#include <cstdlib>                  // For exit
#include <cstring>                  // For strerror, ...

#include "pub/Debug.h"              // For namespace pub::debugging
#include "pub/mutex.h"              // For pub::mutex, implemented
#include "pub/System.h"             // For pub::System::log

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging methods

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
// Production mode settings: HCDM= false; VERBOSE= 0;
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  LL_ERROR= System::LL_ERROR       // Import System::LL_ERROR
}; // generic enum

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static pthread_mutexattr_t
                       mutex_attr{}; // The (common) mutex attribute

//----------------------------------------------------------------------------
// Global initialization/termination
//----------------------------------------------------------------------------
namespace {                         // Anonymous namespace
static struct StaticGlobal {
   StaticGlobal( void )             // Static constructor
{  if( HCDM ) debugf("Mutex::StaticGlobal!\n");

   pthread_mutexattr_init(&mutex_attr);
   pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_NORMAL);
}

   ~StaticGlobal( void )            // Static destructor
{  if( HCDM ) debugf("Mutex::StaticGlobal~\n");

   pthread_mutexattr_destroy(&mutex_attr);
}
}  static_global;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Subroutine-
//       pub::handle_delete_while_busy
//
// Purpose-
//       Isolate pub::mutex delete while busy recovery code
//
//----------------------------------------------------------------------------
void handle_delete_while_busy(pthread_mutex_t*); // Forward reference

void
   handle_delete_while_busy(        // Handle pub::mutex delete while busy
     pthread_mutex_t*  _mutex)      // For this mutex
{
   // We use an analog to an open file destructor: We "close" the mutex,
   // restoring it to its initial state without comment.
   int rc= pthread_mutex_unlock(_mutex);
   if( rc != 0 )                  // (This is an unexpected error, debug it)
     errorf("%4d mutex(%p)::~mutex unlock error(%d:%s)\n", __LINE__, _mutex
           , rc, strerror(rc));
   return;

   /*** Alternatives: ********************************************************
   0) We could do absolutely nothing, not even calling this subroutine.
      This leaves the mutex locked and doesn't delete library resources.
      Also, after the destructor exits, if the user does decide to unlock the
      mutex it's not this mutex any more. It may even be deallocated.

   1) We could simply delay until the user unlocks the mutex:
      (At least gdb would show us stopped in handle_delete_while_busy)

      int rc= pthread_mutex_lock(_mutex);
      if( rc != 0 )
        throwf("pub::mutex(%p)::lock error(%d:%s)\n", _mutex, rc, strerror(rc));

      rc= pthread_mutex_unlock(_mutex);
      if( rc != 0 )
        throwf("mutex(%p)::unlock error(%d:%s)\n", this, rc, strerror(rc));

      return;

   2) We could do nothing, resulting in a spin loop in delete until the user
      unlocks the mutex:
      pthread_yield();              // Give the user a chance to unlock
      return;

   *) We could throw a "don't do this" exception
      throwf("%4d mutex(%p)::~mutex while locked)", __LINE__, _mutex);
   **************************************************************************/
}

//----------------------------------------------------------------------------
// pub::mutex::Constructors/destructor
//----------------------------------------------------------------------------
   mutex::mutex( void )
{  if( HCDM ) debugf("mutex(%p)!\n", this);

   pthread_mutex_init(&_mutex, &mutex_attr);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   mutex::~mutex( void )
{  if( HCDM ) debugf("mutex(%p)~\n", this);

   int rc= pthread_mutex_destroy(&_mutex);
   if( rc == 0 )
     return;

   while( rc == EBUSY ) {
     handle_delete_while_busy(&_mutex); // Handle busy condition
     rc= pthread_mutex_destroy(&_mutex);
   }

   if( rc != 0 )                    // (If error other than EBUSY)
     throwf("%4d mutex(%p)::~mutex destroy error(%d:%s)\n", __LINE__ , this
                , rc, strerror(rc));
}

//----------------------------------------------------------------------------
// pub::mutex::Methods
//----------------------------------------------------------------------------
void
   mutex::lock( void )
{  int rc= pthread_mutex_lock(&_mutex);

   if( rc == 0 )
     return;

   throwf("pub::mutex(%p)::lock error(%d:%s)\n", this, rc, strerror(rc));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool
   mutex::try_lock( void )
{  int rc= pthread_mutex_trylock(&_mutex);

   if( rc == 0 )
     return true;

   if( rc == EBUSY )
     return false;

   throwf("pub::mutex(%p)::try_lock error(%d:%s)\n", this, rc, strerror(rc));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   mutex::unlock( void )
{  int rc= pthread_mutex_unlock(&_mutex);

   if( rc == 0 )
     return;

   throwf("mutex(%p)::unlock error(%d:%s)\n", this, rc, strerror(rc));
}
} // namespace _LIBPUB_NAMESPACE
