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
//       Exception.h
//
// Purpose-
//       Standard Exception and built-in Exceptions.
//
// Last change date-
//       2024/09/29
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_EXCEPTION_H_INCLUDED
#define _LIBPUB_EXCEPTION_H_INCLUDED

#include <stdexcept>                // For std::runtime_error
#include <string>                   // For std::string

#include "Object.h"                 // For Object, _LIBPUB_ macros

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
class Exception : public Object, public std::runtime_error { // Exception base class
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
:  Object(), std::runtime_error(text) {} // (Default) constructor

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
