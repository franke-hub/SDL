//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
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
//       2022/09/02
//
//----------------------------------------------------------------------------
#include <typeinfo>                 // For typeid, used in get_class_name
#include <boost/core/demangle.hpp>  // Used in get_class_name
#include <stdio.h>                  // For sprintf

#include "pub/Object.h"             // For pub::Object, implemented

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <pub/ifmacro.h>

namespace _LIBPUB_NAMESPACE {
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
{  }

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
{  return (char*)this - (char*)(&source); }

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
//       Object::operator std::string
//
// Purpose-
//       Convert to string.
//
//----------------------------------------------------------------------------
   Object::operator std::string( void ) const // Convert to string
{
   char buffer[32];                // Temporary
   sprintf(buffer, "Object@%p", this);
   return buffer;
}
} // namespace _LIBPUB_NAMESPACE
