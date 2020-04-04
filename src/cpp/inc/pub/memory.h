//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the BOOST license,
//       version 1.0.
//       (See accompanying file LICENSE.BOOST-1.0 or the original
//       contained within https://www.boost.org/users/license.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/pub/memory.h
//
// Purpose-
//       TODO: Implement without boost.
//
// Last change date-
//       2018/01/01
//
// Implementation note-
//       This is currently just a wrapper for boost::atomic_shared_ptr.
//       Until this changes, this is distributed under the BOOST license,
//       version 1.0. This license is less restrictive than the LGPL license.
//
//----------------------------------------------------------------------------
#ifndef _PUB_MEMORY_H_INCLUDED
#define _PUB_MEMORY_H_INCLUDED

#define _USE_BOOST_ATOMIC_SHARED_PTR true

#if( _USE_BOOST_ATOMIC_SHARED_PTR )
#include <boost/smart_ptr/atomic_shared_ptr.hpp> // The implementation to beat

#include "config.h"                 // For _PUB_NAMESPACE, ...

namespace _PUB_NAMESPACE {
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
}  // _PUB_NAMESPACE
#else
#error "No other implementation exists for atomic_shared_ptr"
#endif

#undef  _USE_BOOST_ATOMIC_SHARED_PTR // Avoid unnessary defines
#endif // _PUB_MEMORY_H_INCLUDED
