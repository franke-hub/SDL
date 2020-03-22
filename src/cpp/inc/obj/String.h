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
//       String.h
//
// Purpose-
//       Define the String Object.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       Normally std::string works just fine.
//       The String Object is only needed to when you need an Object rather
//       than just a std::string.
//
//----------------------------------------------------------------------------
#ifndef OBJ_STRING_H_INCLUDED
#define OBJ_STRING_H_INCLUDED

#include "Object.h"

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       String
//
// Purpose-
//       The std::string Object
//
//----------------------------------------------------------------------------
class String : public Object, public std::string { // The std::string Object
   using Object::Object;            // Use Object Constructor
   using std::string::string;       // Use std::string Constructors
   using std::string::operator=;    // Use std::string assignment operators

//----------------------------------------------------------------------------
// String::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Result (<0,=0,>0)
   compare(                         // Compare to
     const Object&     source) const // Source
{
   const String* that= dynamic_cast<const String*>(&source);
   if( that == nullptr)
     throw CompareCastException("String");

   return std::string::compare(*that);
}

virtual size_t                      // A hash code value for this Object
   hashf( void ) const              // Create hash code value for Object
{
   std::hash<std::string> builtin;
   return builtin(*this);           // Use std::hash built-in function
}

virtual const std::string           // A String representation of this Object
   string( void ) const             // Represent this Object as a String
{  return *this; }
}; // class String
}  // namespace _OBJ_NAMESPACE

#endif // OBJ_STRING_H_INCLUDED
