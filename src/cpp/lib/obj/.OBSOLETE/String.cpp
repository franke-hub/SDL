//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       String.cpp
//
// Purpose-
//       String implementation methods. (Now in String.h)
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include "obj/String.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef OBJECT_STATISTICS
#define OBJECT_STATISTICS           // If defined, update Object::objectCount
#endif

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
//
// Method-
//       String::compare
//
// Purpose-
//       Compare to.
//
//----------------------------------------------------------------------------
int                                 // Result (<0,=0,>0)
   String::compare(                 // Compare to
     const Object&     source) const // This Object
{
   const String* that= dynamic_cast<const String*>(&source);
   if( that == nullptr )
     throw DynamicCastException();

#if true
   return std::string::compare(*that);
#else
   const unsigned char* L= (const unsigned char*)(this->c_str());
   const unsigned char* R= (const unsigned char*)(that->c_str());

   for(;;)
   {
     if( *L != *R )
       break;

     if( *L == '\0' )
       break;

     L++;
     R++;
   }

   return *L - *R;
#endif
}

#if true
//----------------------------------------------------------------------------
//
// Method-
//       String::hashf
//
// Purpose-
//       std::hash<std::string> Hash function.
//
//----------------------------------------------------------------------------
static std::hash<std::string> hash_builtin;

size_t                              // Resultant
   String::hashf( void ) const      // Hash function
{
   return hash_builtin(*this);      // Use built-in hash function
}
#else
//----------------------------------------------------------------------------
//
// Method-
//       String::hashf
//
// Purpose-
//       (DJB2) Hash function.
//
//----------------------------------------------------------------------------
size_t                              // Resultant
   String::hashf( void ) const      // Hash function
{
   size_t hash= 5381;               // Initial (pragmatic) value
   const unsigned char* C= (const unsigned char*)c_str(); // The string
   while( *C != '\0' )
   {
     hash= (hash << 5) + hash + (*C++); // hash * 33 + (*C++)
   }

   return hash;
}
#endif
} // namespace _OBJ_NAMESPACE

