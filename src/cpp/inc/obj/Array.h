//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Array.h
//
// Purpose-
//       Array of Object References.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef OBJ_ARRAY_H_INCLUDED
#define OBJ_ARRAY_H_INCLUDED
#include <array>

#include "Object.h"                 // Ref instances refer to Objects
#include "Exception.h"              // Exceptions can be thrown

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       Array_t<class T, std::size_t N>
//
// Purpose-
//       An array of Ref_t<T> Object references
//
//----------------------------------------------------------------------------
template<class T, std::size_t N>
class Array_t : public Object, public std::array<Ref_t<T>, N> { // Reference array
   using Object::Object;
   using std::array<Ref_t<T>, N>::array;
}; // class Array_t

//----------------------------------------------------------------------------
//
// Class-
//       Array
//
// Purpose-
//       An array of Object references.
//
//----------------------------------------------------------------------------
template<std::size_t N>
class Array : public Object, public std::array<Ref, N> { // Object reference array
   using Object::Object;
   using std::array<Ref, N>::array;
}; // class Array
}  // namespace _OBJ_NAMESPACE

#endif // OBJ_ARRAY_H_INCLUDED
