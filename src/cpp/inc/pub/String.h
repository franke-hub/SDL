//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2024 Frank Eskesen.
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
//       A std::string container Object
//
// Last change date-
//       2024/09/30
//
// Implementation notes-
//       String.h only implements Object.h methods, but does provide implicit
//       conversion into a std::string from a pub::String.
//
//       Conversion into a std::string& (modifiable reference) can be done
//       explicity, e.g.:
//         std::string= ( pub::String.get_string() + "abc" );
//         std::string= ( (std::string&)pub::String + "abc" );
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_STRING_H_INCLUDED
#define _LIBPUB_STRING_H_INCLUDED

#include <string>                   // For std::string

#include "pub/Object.h"             // For pub::Object, implemented

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       String
//
// Purpose-
//       An Object *containing* a std::string
//
// Implementation note-
//       *Experimental* and while similar is NOT exactly a std::string
//
//----------------------------------------------------------------------------
class String : public Object {
//----------------------------------------------------------------------------
// String::Attributes
//----------------------------------------------------------------------------
protected:
std::string            _str;        // Our std::string

//----------------------------------------------------------------------------
// String::Constructors/destructor
//----------------------------------------------------------------------------
public:
   String( void ) = default;        // Default constructor
   String(const Object& obj)        // Object copy constructor
{  _str= obj.to_string(); }         // NOTE: Might not be what you'd expect

   String(const char* str)          // Copy constructor (from const char*)
{  _str= str; }                     // (What you'd expect)

   String(const std::string& str)   // Copy constructor (from std::string)
{  _str= str; }                     // (What you'd expect)

//----------------------------------------------------------------------------
// String::Accessor methods
//----------------------------------------------------------------------------
std::string&                        // The ACTUAL _str
   get_string( void )               // Get the ACTUAL _str
{  return _str; }

//----------------------------------------------------------------------------
// String::operators
//----------------------------------------------------------------------------
   operator std::string( void ) const // (Implicit) Cast operator
{  return _str; }

   explicit operator std::string&( void ) // (Explicit) Cast operator
{  return _str; }

Object&
   operator=(const Object obj)      // Assignment operator
{  _str= obj.to_string();           // NOTE: Might not be what you'd expect
   return *this;
}

Object&
   operator=(const std::string str) // Assignment operator
{  _str= str;                       // What you'd expect
   return *this;
}

bool
   operator==(const std::string& obj) const // Test for equality
{  return _str == obj; }

//----------------------------------------------------------------------------
// String::Object methods
//----------------------------------------------------------------------------
virtual int                         // Result (<0,=0,>0)
   compare(                         // Compare to
     const Object&     obj) const   // This Object&
{
   std::string str= obj.to_string(); // NOTE: Might not be what you'd expect
   if( _str < str )
     return -1;
   else if( _str > str )
     return +1;

   return 0;
}

virtual size_t                      // The Object hash value
   hashf( void ) const              // Get hash code value
{  return std::hash<std::string>{}(_str); }

std::string                         // (A COPY of) _str
   to_string( void ) const          // Get String representation of this Object
{  return _str; }
}; // class String
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_STRING_H_INCLUDED
