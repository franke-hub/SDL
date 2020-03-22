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
//       Object.cpp
//
// Purpose-
//       Object method implementations.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <typeinfo>                 // For typeid, used in get_class_name
#include <boost/core/demangle.hpp>  // Used in get_class_name
#include <com/Debug.h>              // For debugging

#include "obj/Object.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <obj/ifmacro.h>

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
//
// Method-
//       Object::~Object
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Object::~Object( void )          // Destructor
{
   int32_t count= references;       // Get (instantaneous) reference count
   if( count != 0 )
     Exception::abort("Object(%p)::~Object, references(%d)\n", this, count);
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::compare
//
// Purpose-
//       Compare to.
//
//----------------------------------------------------------------------------
int                                 // Result (<0,=0,>0)
   Object::compare(                 // Compare to
     const Object&     source) const // This Object
{
   return (char*)this - (char*)(&source);
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::hashf
//
// Purpose-
//       Hash function.
//
//----------------------------------------------------------------------------
size_t                              // Resultant
   Object::hashf( void ) const      // Hash function
{
   uintptr_t result= uintptr_t(this);
   result >>= 3;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::get_class_name
//
// Purpose-
//       Return the class name
//
//----------------------------------------------------------------------------
std::string
   Object::get_class_name( void ) const // Get the class name
{
   const char* mangled= typeid(*this).name();
   return boost::core::demangle(mangled);
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::string
//
// Purpose-
//       Convert to string.
//
//----------------------------------------------------------------------------
const std::string                   // Resultant
   Object::string( void ) const     // Convert to string
{
   return _OBJ_NAMESPACE::built_in::to_string("Object@%p", this);
}
} // namespace _OBJ_NAMESPACE

