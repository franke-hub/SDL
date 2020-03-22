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
//       SampleFactory.h
//
// Purpose-
//       Define a SampleFactory object.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef SAMPLEFACTORY_H_INCLUDED
#define SAMPLEFACTORY_H_INCLUDED

#ifndef FACTORY_H_INCLUDED
#include "Factory.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       SampleFactory
//
// Purpose-
//       Sample Factory
//
//----------------------------------------------------------------------------
class SampleFactory : public Factory { // Sample Factory
//----------------------------------------------------------------------------
//
// Class-
//       SampleFactory::Object
//
// Purpose-
//       A SampleFactory::Object is an Object produced by a SampleFactory
//
//----------------------------------------------------------------------------
public:
class Object : public Interface {   // SampleFactory::Object
//----------------------------------------------------------------------------
// SampleFactory::Object::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Object( void );                 // Destructor
   Object( void );                  // Default constructor

//----------------------------------------------------------------------------
// SampleFactory::Object::Methods
//----------------------------------------------------------------------------
virtual void
   doSomething(                     // We have to do something, after all
     const char*       message) const; // Let's print a message
}; // class SampleFactory::Object

//----------------------------------------------------------------------------
// SampleFactory::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~SampleFactory( void );          // Destructor
   SampleFactory( void );           // Default constructor

private:                            // Bitwise copy is prohibited
   SampleFactory(const SampleFactory&); // Disallowed copy constructor
SampleFactory&
   operator=(const SampleFactory&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// SampleFactory::Methods (SampleFactory IS the object)
//----------------------------------------------------------------------------
public:
virtual Interface*                  // Resultant SampleFactory::Object
   make( void );                    // Create an Interface Object

virtual void                        // Method MUST BE virtual
   talk(                            // Write (printf)
     const char*       message) const; // This test message
}; // class SampleFactory

#endif // SAMPLEFACTORY_H_INCLUDED
