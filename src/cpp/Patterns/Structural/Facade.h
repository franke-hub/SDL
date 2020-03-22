//----------------------------------------------------------------------------
//
//       Copyright (c) 2017 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Facade.h
//
// Purpose-
//       Provide a unified set of interfaces to a subsystem.
//       Isolate the internal subsystem interfaces from a client.
//
// Last change date-
//       2017/01/01
//
// Also known as-
//       Wrapper.
//
//----------------------------------------------------------------------------
#ifndef FACADE_H_INCLUDED
#define FACADE_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       Facade
//
// Purpose-
//       Define the Facade object.
//
//----------------------------------------------------------------------------
class Facade : public Object
{
//----------------------------------------------------------------------------
// Facade::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Facade( void );
   Facade( void );

//----------------------------------------------------------------------------
// Facade::Methods
//----------------------------------------------------------------------------
public:
virtual void
   function1( void );

virtual void
   function2( void );

virtual void
   function3( void );

virtual void
   function4( void );

//----------------------------------------------------------------------------
// Facade::Attributes
//----------------------------------------------------------------------------
protected:
   Internal1*          internal1;
   Internal2*          internal2;
   Internal3*          internal3;
   Internal4*          internal4;
   Internal5*          internal5;
   Internal6*          internal6;
   Internal7*          internal7;
   Internal8*          internal8;
}; // class Facade
#endif  // FACADE_H_INCLUDED
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//
// Class-
//       SampleClient
//
// Purpose-
//       Use the Decorator.
//
//----------------------------------------------------------------------------
class SampleClient : public Object
{
void*
   run( void )
{
   Facade*                facade= new Facade();

   facade->function1();
   facade->function2();
   facade->function3();
   facade->function4();
} // void run
} // class SampleClient

