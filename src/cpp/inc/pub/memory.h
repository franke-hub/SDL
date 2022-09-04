//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2022 Frank Eskesen.
//
//       This file is free content, distributed under the BOOST license,
//       version 1.0.
//       (See accompanying file LICENSE.BOOST-1.0 or the original
//       contained within https://www.boost.org/users/license.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       memory.h
//
// Purpose-
//       Define atomic_shared_ptr
//
// Last change date-
//       2022/09/02
//
// Implementation note-
//       TODO: Implement without boost.
//       This is currently just a wrapper for boost::atomic_shared_ptr.
//       Until this changes, this is distributed under the BOOST license,
//       version 1.0. This license is less restrictive than the LGPL license.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_MEMORY_H_INCLUDED
#define _LIBPUB_MEMORY_H_INCLUDED

#define _USE_BOOST_ATOMIC_SHARED_PTR true

#if( _USE_BOOST_ATOMIC_SHARED_PTR )
#include <boost/smart_ptr/atomic_shared_ptr.hpp> // The implementation to beat

// Even though _LIBPUB_ macros are unused we still need this include.
// It's needed here to follow the rule that any and all pub include files
// directly or indirectly include pub/bits/pubconfig.h
#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros

//----------------------------------------------------------------------------
//
// Class-
//       atomic_shared_ptr<class T>
//
// Purpose-
//       Atomic shared pointer.
//
//----------------------------------------------------------------------------
template<class T>
class atomic_shared_ptr             // Atomic shared pointer
:  public boost::atomic_shared_ptr<T> {
// using boost::atomic_shared_ptr<T>; // Templates invalid in using-declaration
// Constructors
   atomic_shared_ptr()
:  boost::atomic_shared_ptr<T>() {};

   atomic_shared_ptr(boost::shared_ptr<T> p)
:  boost::atomic_shared_ptr<T>(p) {};

   atomic_shared_ptr(std::shared_ptr<T> p) // Probably not needed
:  boost::atomic_shared_ptr<T>(p) {};
}; // class atomic_shared_ptr<>
#else
#error "No other implementation exists for atomic_shared_ptr"
#endif

#undef  _USE_BOOST_ATOMIC_SHARED_PTR // Avoid unnessary defines
#endif // _LIBPUB_MEMORY_H_INCLUDED
