//----------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       mutex.h
//
// Purpose-
//       Definition and implementation of pub:mutex struct
//
// Last change date-
//       2026/03/23
//
// Implementation notes-
//       This mutex can be locked by one thread and unlocked by another.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_MUTEX_H_INCLUDED
#define _LIBPUB_MUTEX_H_INCLUDED

#include <mutex>                    // For std::lock_guard, ...
#include <cerrno>                   // For errno

#include <pthread.h>                // For pthread_cond_t, pthread_mutex_t, ...

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       pub::mutex
//
// Purpose-
//       Minimal mutex descriptor
//
// Implementation notes-
//       We use an *unchecked* pthread_mutex_t
//       This allows (doesn't check for) unlock by a different thread.
//
//----------------------------------------------------------------------------
class mutex {                       // The Mimimal mutex descriptor
//----------------------------------------------------------------------------
// pub::mutex::Attributes
//----------------------------------------------------------------------------
protected:
pthread_mutex_t        _mutex=      // The mutex
                         PTHREAD_MUTEX_INITIALIZER;

//----------------------------------------------------------------------------
// pub::mutex::Constructors/Destructor
//----------------------------------------------------------------------------
public:
   mutex( void );

   ~mutex( void );

// Disallowed: Copy constructor, assignment operator
   mutex(const mutex&) = delete;
mutex& operator=(const mutex&) = delete;

//----------------------------------------------------------------------------
// pub::mutex::Methods
//----------------------------------------------------------------------------
void
   lock( void );                    // Obtain the mutex

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool
   try_lock( void );                // Conditionally obtain the mutex

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   unlock( void );                  // Release the mutex
}; // class pub::mutex
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_MUTEX_H_INCLUDED
