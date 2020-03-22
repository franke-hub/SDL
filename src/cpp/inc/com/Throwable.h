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
//       Throwable.h
//
// Purpose-
//       Throwable is the base throw class.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef THROWABLE_H_INCLUDED
#define THROWABLE_H_INCLUDED

#include <exception>

//----------------------------------------------------------------------------
//
// Class-
//       Throwable
//
// Purpose-
//       Base target class for throw.
//
//----------------------------------------------------------------------------
class Throwable : public std::exception { // Generic Throwable
//----------------------------------------------------------------------------
// Throwable::Attributess
//----------------------------------------------------------------------------
protected:
// None defined

//----------------------------------------------------------------------------
// Throwable::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Throwable( void )               // Destructor
   throw() {}

   Throwable( void )                // Constructor
   throw()
:  std::exception() {}

   Throwable(                       // Copy constructor
     const Throwable&  source)      // Source Throwable&
   throw()
:  std::exception(source) {}

//----------------------------------------------------------------------------
// Throwable::Methods
//----------------------------------------------------------------------------
public:
virtual                             // Exception description
   operator const char*( void ) const // operator (const char*)
   throw()                          // Guaranteed nothrow
{  return what(); }
}; // class Throwable

#endif // THROWABLE_H_INCLUDED
