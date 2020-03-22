//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Factory.h
//
// Purpose-
//       Define the Factory object.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef FACTORY_H_INCLUDED
#define FACTORY_H_INCLUDED

#ifndef INTERFACE_H_INCLUDED
#include "Interface.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Factory
//
// Purpose-
//       Factory
//
// Implementation notes-
//       Factory::make may throw(const char*) rather than return NULL.
//
//----------------------------------------------------------------------------
class Factory : public Interface {  // Factory
//----------------------------------------------------------------------------
// Factory::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Factory( void );                // Destructor
   Factory( void );                 // Default constructor

private:                            // Bitwise copy is prohibited
   Factory(const Factory&);         // Disallowed copy constructor
Factory&
   operator=(const Factory&);       // Disallowed assignment operator

//----------------------------------------------------------------------------
// Factory::Methods (Derived Factory::make methods may have parameters.)
//----------------------------------------------------------------------------
public:
virtual Interface*                  // Resultant Interface Object
   make( void );                    // Create an Interface Object

virtual void
   take(                            // Recycle
     Interface*        object);     // This Interface Object
}; // class Factory

#endif // FACTORY_H_INCLUDED
