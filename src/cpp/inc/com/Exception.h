//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2014 Frank Eskesen.
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
//       Exception base class.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef EXCEPTION_H_INCLUDED
#define EXCEPTION_H_INCLUDED

#include <string>

#ifndef THROWABLE_H_INCLUDED
#include "Throwable.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Exception
//
// Purpose-
//       Exception base class.
//
//----------------------------------------------------------------------------
class Exception : public Throwable { // Exception base class
//----------------------------------------------------------------------------
// Exception::Attributes
//----------------------------------------------------------------------------
public:
std::string            text;        // Exception string

//----------------------------------------------------------------------------
// Exception::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Exception( void ) throw() {}    // Destructor

   Exception( void ) throw()        // Default constructor
:  Throwable(), text("Exception") {}

   Exception(                       // Constructor
     const char*       text)        // String
   throw()
:  Throwable(), text(text) {}

   Exception(                       // Copy constructor
     const Exception&  source)      // Source Exception&
   throw()
:  Throwable(source), text(source.text) {}

//----------------------------------------------------------------------------
// Exception::Methods
//----------------------------------------------------------------------------
public:
virtual const char*
   what( void ) const throw()
{  return text.c_str(); }

//----------------------------------------------------------------------------
// Exception::Static methods
//----------------------------------------------------------------------------
public:
static void
   backtrace( void );               // Display backtrace information
}; // struct Exception

//----------------------------------------------------------------------------
//
// Example classes-
//       ConstructionException
//       NoStorageException
//       NotImplementedException
//       ParameterException
//       SystemResourceException
//
// Purpose-
//       Predefined Exception classes.
//
//----------------------------------------------------------------------------
class ConstructionException : public Exception {
public:
   ConstructionException(
     const char*       text= "ConstructionException") throw()
:  Exception(text) {}
}; // class ConstructionException

class NoStorageException : public Exception {
public:
   NoStorageException(
     const char*       text= "NoStorageException") throw()
:  Exception(text) {}
}; // class NoStorageException

class NotImplementedException : public Exception {
public:
   NotImplementedException(
     const char*       text= "NotImplementedException") throw()
:  Exception(text) {}
}; // class NotImplementedException

class ParameterException : public Exception {
public:
   ParameterException(
     const char*       text= "ParameterException") throw()
:  Exception(text) {}
}; // class ParameterException

class SystemResourceException : public Exception {
public:
   SystemResourceException(
     const char*       text= "SystemResourceException") throw()
:  Exception(text) {}
}; // class SystemResourceException

#endif // EXCEPTION_H_INCLUDED
