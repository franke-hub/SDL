//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2019 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       built_in.h
//
// Purpose-
//       Defines the BASIC built-in functions and global oprerators.
//
// Last change date-
//       2019/03/16
//
// Implementation notes-
//       Non-basic global operators may be defined elsewhere.
//       Only functions required by the core objects are included here.
//
// Global operators defined here-
//       bool operator==(obj::Object&, obj::Object&)
//       bool operator!=(obj::Object&, obj::Object&)
//       bool operator<=(obj::Object&, obj::Object&)
//       bool operator>=(obj::Object&, obj::Object&)
//       bool operator< (obj::Object&, obj::Object&)
//       bool operator> (obj::Object&, obj::Object&)
//       std::ostream operator<<(std::ostream&. const obj::Object&)
//       std::ostream operator<<(std::ostream&. const obj::Ref&)
//
//----------------------------------------------------------------------------
#ifndef OBJ_BUILT_IN_H_INCLUDED
#define OBJ_BUILT_IN_H_INCLUDED

#include <ostream>                  // For global operators
#include <stdarg.h>                 // For to_string
#include <stdio.h>                  // For vsnprintf

#include "Object.h"
#include "Exception.h"
#include "Ref.h"

namespace _OBJ_NAMESPACE::built_in {
//----------------------------------------------------------------------------
//
// Subroutine-
//       to_string
//
// Purpose-
//       Create string from printf format arguments
//
//----------------------------------------------------------------------------
static inline std::string           // Resultant
   to_string(                       // Create string from printf arguments
     const char*       fmt,         // Format string
                       ...)         // PRINTF arguments
   _ATTRIBUTE_PRINTF(1, 2);

static inline std::string           // Resultant
   to_stringv(                      // Create string from printf arguments
     const char*       fmt,         // Format string
     va_list           args)        // PRINTF arguments
   _ATTRIBUTE_PRINTF(1, 0);

static inline std::string           // Resultant
   to_string(                       // Create string from printf arguments
     const char*       fmt,         // Format string
                       ...)         // PRINTF arguments
{
   va_list args;
   va_start(args, fmt);
     std::string result= to_stringv(fmt, args);
   va_end(args);

   return result;
}

static inline std::string           // Resultant
   to_stringv(                      // Create string from printf arguments
     const char*       fmt,         // Format string
     va_list           args)        // PRINTF arguments
{
   std::string         result;      // Resultant

   char autobuff[512];              // Working buffer
   char* buffer= autobuff;          // -> Buffer

   va_list copy;
   va_copy(copy, args);
   unsigned L= vsnprintf(buffer, sizeof(autobuff), fmt, copy);
   va_end(copy);

   if( L < sizeof(autobuff) )       // If the normal case
     result= std::string(buffer);
   else
   {
     buffer= new char[L+1];
     vsnprintf(buffer, L, fmt, args);
     result= std::string(buffer);
     delete [] buffer;
   }

   return result;
}
} // namespace _OBJ_NAMESPACE::built_in

//----------------------------------------------------------------------------
//
// Description-
//       Global operators
//
// Purpose-
//       Global comparison operators, global cout << operator.
//
//----------------------------------------------------------------------------
#define _OBJ _OBJ_NAMESPACE
inline bool                         // Resultant
   operator==(                      // Compare (L::R) for equality
     _OBJ::Object&     L,           // Left parameter
     _OBJ::Object&     R)           // Right paramaeter
{  return (L.compare(R) == 0); }

inline bool                         // Resultant
   operator!=(                      // Compare (L::R) for inequality
     _OBJ::Object&     L,           // Left parameter
     _OBJ::Object&     R)           // Right paramaeter
{  return (L.compare(R) != 0); }

inline bool                         // Resultant
   operator<=(                      // Compare (L::R) for lesser || equality
     _OBJ::Object&     L,           // Left parameter
     _OBJ::Object&     R)           // Right paramaeter
{  return (L.compare(R) <= 0);
}

inline bool                         // Resultant
   operator>=(                      // Compare (L::R) for greater || equality
     _OBJ::Object&     L,           // Left parameter
     _OBJ::Object&     R)           // Right paramaeter
{  return (L.compare(R) >= 0); }

inline bool                         // Resultant
   operator<(                       // Compare (L::R) for lesser
     _OBJ::Object&     L,           // Left parameter
     _OBJ::Object&     R)           // Right paramaeter
{  return (L.compare(R) < 0); }

inline bool                         // Resultant
   operator>(                       // Compare (L::R) for greater
     _OBJ::Object&      L,          // Left parameter
     _OBJ::Object&      R)          // Right paramaeter
{  return (L.compare(R) > 0); }

inline std::ostream&                // (stream)
   operator<<(                      // Append to output stream
     std::ostream&     stream,      // (This stream)
     const _OBJ::Object&object)     // (This _OBJ::Object)
{  return stream << object.string(); }

inline std::ostream&                // (stream)
   operator<<(                      // Append to output stream
     std::ostream&     stream,      // (This stream)
     const _OBJ::Ref&   ref)        // (This Ref<_OBJ::Object>)
{  return stream << ref.use().string(); }
#undef _OBJ

#endif // OBJ_BUILT_IN_H_INCLUDED
