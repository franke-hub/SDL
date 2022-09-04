//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Mutex.h
//
// Purpose-
//       Define the Mutex Object, combining std::mutex and Object
//
// Last change date-
//       2022/09/02
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_MUTEX_H_INCLUDED
#define _LIBPUB_MUTEX_H_INCLUDED

#include <mutex>                    // For std::mutex base class
#include <pub/Object.h>             // For pub::Object, base class

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       Mutex
//
// Purpose-
//       A std::mutex Object.
//
// Implementation notes-
//       No Object methods are overridden.
//
//       No std::mutex methods are overriddern.
//         lock(), try_lock(), unlock(), and native_handle()
//       Used with lock_guard exactly like a std::mutex
//         using _LIBPUB_NAMESPACE::Mutex
//         Mutex instance;
//         std::lock_guard<decltype(instance)> lock(instance);
//
//----------------------------------------------------------------------------
class Mutex : public std::mutex, public Object { // The Mutex Object
//----------------------------------------------------------------------------
// Mutex::Constructors/Destructors
//----------------------------------------------------------------------------
public:
virtual
   ~Mutex( void ) {};
   Mutex( void ) : std::mutex(), Object() {};

// Disallowed: Copy constructor, assignment operator
   Mutex(const Mutex&) = delete;
Mutex& operator=(const Mutex&) = delete;
}; // class Mutex
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_MUTEX_H_INCLUDED
