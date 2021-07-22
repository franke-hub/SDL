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
//       Exception.h
//
// Purpose-
//       Define the (local) standard exceptions
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef EXCEPTION_H_INCLUDED
#define EXCEPTION_H_INCLUDED

#include <boost/core/demangle.hpp>
#include <stdexcept>
#include <typeinfo>

//----------------------------------------------------------------------------
//
// Class-
//       Exception
//
// Purpose-
//       The Exception base class.
//
// Implementation notes-
//       std::runtime_error rather than std::exception is the Exception base
//       class because there is no std::exception(std::string) constructor.
//
//----------------------------------------------------------------------------
class Exception : public std::runtime_error {
   using std::runtime_error::runtime_error;

   public:
   virtual std::string
     get_class_name( void ) const { // Get the class name
       const char* mangled= typeid(*this).name();
       return std::string(boost::core::demangle(mangled));
   }

   inline std::string               // ClassName(descriptor)
     get_class_what() const {       // Get descriptor with class name wrapper
       std::string result= get_class_name();
       result += "(";
       result += what();
       result += ")";
       return result;
   }

   // Constructors, from std::runtime_error
   // Exception(const std::string&)
   // Exception(const char*)
   // Exception(const std::runtime_error&), allows Exception(const Exception&)
};

//----------------------------------------------------------------------------
//
// Class-
//       KeyError
//       NoStorageException
//       NotImplementedException
//       NullPointerException
//
// Purpose-
//       The Built-in Exception classes.
//
// Implementation notes-
//       We do not provide the RuntimeException class because the Exception
//       class is derived from std::runtime_error.
//
//----------------------------------------------------------------------------
class KeyError : public Exception {
   using Exception::Exception;
};

class NoStorageException : public Exception {
   using Exception::Exception;
};

class NotImplementedException : public Exception {
   using Exception::Exception;
};

class NullPointerException : public Exception {
   using Exception::Exception;
};

#endif // EXCEPTION_H_INCLUDED
