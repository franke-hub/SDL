//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2026 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Exception.h
//
// Purpose-
//       Standard Exception and built-in Exceptions.
//
// Last change date-
//       2026/09/07
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_EXCEPTION_H_INCLUDED
#define _LIBPUB_EXCEPTION_H_INCLUDED

#include <stdexcept>                // For std::runtime_error
#include <string>                   // For std::string
#include <typeinfo>                 // For std::type_info

#include <pub/utility.h>            // For pub::demangle

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       Exception
//
// Purpose-
//       Exception base class.
//
//----------------------------------------------------------------------------
class Exception : public std::runtime_error { // Exception base class
//----------------------------------------------------------------------------
// Exception::Attributes
//----------------------------------------------------------------------------
protected:
static constexpr const char* default_exception= "Exception";

//----------------------------------------------------------------------------
// Exception::Constructors
//----------------------------------------------------------------------------
public:
inline
   Exception(                       // String constructor
     const std::string text= default_exception) // Exception descriptor
:  std::runtime_error(text)         // (Default) constructor
{  }

//----------------------------------------------------------------------------
// Exception::Object methods
//----------------------------------------------------------------------------
virtual inline
   explicit operator std::string( void ) const // String extractor
{
   std::string result;

   try {
     result= get_class_name() + "(" + what() + ")";
   } catch(...) {
     result= default_exception;
   }

   return result;
}

std::string
   get_class_name() const
{  return pub::utility::demangle(typeid(*this)); }

std::string
   to_string() const
{  return operator std::string(); }
}; // class Exception

//----------------------------------------------------------------------------
//
// Class-
//       IndexException             // Invalid index
//
// Purpose-
//       Exception built-in classes
//
//----------------------------------------------------------------------------
// IndexException: When getting: index unknown. When setting: index exists.
class IndexException : public Exception { using Exception::Exception;
}; // class IndexException
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_EXCEPTION_H_INCLUDED
