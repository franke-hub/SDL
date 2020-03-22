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
//       Flyweight.h
//
// Purpose-
//       Use sharing to efficiently support a large number of fine-grained
//       objects.  It can be used in multiple Contexts simultaneously.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef FLYWEIGHT_H_INCLUDED
#define FLYWEIGHT_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       State
//
// Purpose-
//       Define a State object.
//
//----------------------------------------------------------------------------
class State : public Object
{
//----------------------------------------------------------------------------
// State::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~State( void );
   State( void );

//----------------------------------------------------------------------------
// State::Methods
//----------------------------------------------------------------------------
   // None defined

//----------------------------------------------------------------------------
// State::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class State

//----------------------------------------------------------------------------
//
// Class-
//       Flyweight
//
// Purpose-
//       Define the Flyweight object.
//
//----------------------------------------------------------------------------
class Flyweight : public Object
{
//----------------------------------------------------------------------------
// Flyweight::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Flyweight( void );

protected:
   Flyweight( void );

//----------------------------------------------------------------------------
// Flyweight::Methods
//----------------------------------------------------------------------------
public:
static Flyweight*                   // The associated instance
   getInstance(                     // Get instance
     unsigned          key);        // Using this key

virtual void
   function1( void );

virtual void
   function2(
     State&            state);

//----------------------------------------------------------------------------
// Flyweight::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class Flyweight
#endif  // FLYWEIGHT_H_INCLUDED
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
   State               state();
   Flyweight*          flyweight;

   for(int i= 0; i<1000; i++)
   {
     flyweight= Flyweight::getInstance(i%10);
     flyweight->function1();
     flyweight->function2(state);
   }
} // void run
} // class SampleClient

