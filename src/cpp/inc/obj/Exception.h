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
//       Exception.h
//
// Purpose-
//       Standard Exception and built-in Exceptions.
//
// Usage notes-
//       Include Object.h to include this file and avoid circular dependeny
//       errors.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef OBJ_EXCEPTION_H_INCLUDED
#define OBJ_EXCEPTION_H_INCLUDED

#include <stdexcept>

#include "Object.h"

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       Exception
//
// Purpose-
//       Exception base class.
//
// Implementation notes-
//       Like std::runtime_error, we do not define a default constructor.
//
//----------------------------------------------------------------------------
class Exception : public Object, public std::runtime_error { // Exception base class
   using Object::Object;
   using std::runtime_error::runtime_error;

//----------------------------------------------------------------------------
// Exception::Attributes
//----------------------------------------------------------------------------
protected:
static std::string     default_exception_string; // "Exception"

//----------------------------------------------------------------------------
// Exception::Constructors
//----------------------------------------------------------------------------
public:
inline
   Exception( void )                // Default constructor
:  Object(), std::runtime_error("") {}

//----------------------------------------------------------------------------
// Exception::Object methods
//----------------------------------------------------------------------------
virtual inline const std::string
   string( void ) const             // String extractor
{
   std::string result;

   try {
     result= get_class_name() + "(" + what() + ")";
   } catch(...) {
     result= default_exception_string;
   }

   return result;
}

//----------------------------------------------------------------------------
// Exception::Methods
//----------------------------------------------------------------------------
static int                          // Indicate failure. (Normally no return)
   abort(                           // Write to trace and stderr, throw exception
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(1, 2);
}; // class Exception

//----------------------------------------------------------------------------
//
// Class-
//       CompareCastException       // Cannot compare dissimilar Objects
//       NoStorageException         // Storage allocation failure
//       NullPointerException       // NULL pointer detected
//
// Purpose-
//       Exception built-in classes
//
//----------------------------------------------------------------------------
class CompareCastException : public Exception { using Exception::Exception;
}; // class CompareCastException

class NoStorageException : public Exception { using Exception::Exception;
}; // class NoStorageException

class NullPointerException : public Exception { using Exception::Exception;
}; // class NullPointerException
}  // _OBJ_NAMESPACE

#endif // OBJ_EXCEPTION_H_INCLUDED
