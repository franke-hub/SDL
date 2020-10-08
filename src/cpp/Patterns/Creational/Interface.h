//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Interface.h
//
// Purpose-
//       Define a class used to describe what an Object can do.
//       There may be multiple different implentations of an Interface.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef INTERFACE_H_INCLUDED
#define INTERFACE_H_INCLUDED
#include ".Object/Object.h"         // For examples

//----------------------------------------------------------------------------
//
// Class-
//       Interface
//
// Purpose-
//       Defines an interface for duplicating Objects which allows subclasses
//       decide which class to instantiate.
//
//----------------------------------------------------------------------------
class Interface {                   // Generally has no base class
//----------------------------------------------------------------------------
// Interface::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
// None defined

//----------------------------------------------------------------------------
// Interface::Attributes
//----------------------------------------------------------------------------
protected:
// None defined

//----------------------------------------------------------------------------
// Interface::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Interface( void ) = 0;          // PURE VIRTUAL destructor
// Interface( void );               // NO constructor

//----------------------------------------------------------------------------
// Interface::Methods
//----------------------------------------------------------------------------
public:
virtual int
   doSomething( void ) = 0;         // PURE VIRTUAL method
}; // class Interface

#endif  // INTERFACE_H_INCLUDED
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// Implementation: GCC code WILL reference this!
   Interface::~Interface( void ) {} // Instantiate pure virtual method

//----------------------------------------------------------------------------
//
// Title-
//       SampleInterface.h
//
// Purpose-
//       Sample Interface.
//
// Concrete Classes-
         class SampleInterface1;
         class SampleInterface2;
//
//----------------------------------------------------------------------------
#ifndef SAMPLEINTERFACE_H_INCLUDED
#define SAMPLEINTERFACE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       SampleInterface1
//
// Purpose-
//       Sample Interface implementation.
//
//----------------------------------------------------------------------------
class SampleInterface1 : public Object, virtual public Interface {
//----------------------------------------------------------------------------
// SampleInterface1::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
// None defined

//----------------------------------------------------------------------------
// SampleInterface1::Attributes
//----------------------------------------------------------------------------
protected:
// None defined

//----------------------------------------------------------------------------
// SampleInterface1::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleInterface1( void ) {}
   SampleInterface1( void ) : Object() {};

//----------------------------------------------------------------------------
// SampleInterface1::Interface implementation methods
//----------------------------------------------------------------------------
public:
virtual int
   doSomething( void )              // Implements Interface
{
   printf("SAMPLEINTERFACE1 does something\n");
   return 0;
}
}; // class SampleInterface1

//----------------------------------------------------------------------------
//
// Class-
//       SampleInterface2
//
// Purpose-
//       Sample Interface implementation.
//
//----------------------------------------------------------------------------
class SampleInterface2 : public Object, virtual public Interface {
//----------------------------------------------------------------------------
// SampleInterface2::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
// None defined

//----------------------------------------------------------------------------
// SampleInterface2::Attributes
//----------------------------------------------------------------------------
protected:
// None defined

//----------------------------------------------------------------------------
// SampleInterface2::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleInterface2( void ) {};
   SampleInterface2( void ) : Object() {};

//----------------------------------------------------------------------------
// SampleInterface2::Interface implementation methods
//----------------------------------------------------------------------------
public:
virtual int
   doSomething( void )              // Implements Interface
{
   printf("SAMPLEINTERFACE2 does something else\n");
   return 0;
}
}; // class SampleInterface2
#endif  // SAMPLEINTERFACE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       InterfaceSampleClient
//
// Purpose-
//       Use the SampleInterface.
//
//----------------------------------------------------------------------------
class InterfaceSampleClient : public Object {
public:
void
   run( void )
{
   Interface*          one = new SampleInterface1();
   Interface*          two = new SampleInterface2();

   one->doSomething();
   two->doSomething();

   delete one;
   delete two;
} // void run
}; // class InterfaceSampleClient

