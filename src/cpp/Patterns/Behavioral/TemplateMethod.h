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
//       TemplateMethod.h
//
// Purpose-
//       Define the skeleton of an algorithm in an operation, deferring
//       some steps to subclasses.  TemplateMethod lets subclasses redefine
//       certain steps without changing the algorithm's structure.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef TEMPLATEMETHOD_H_INCLUDED
#define TEMPLATEMETHOD_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       AbstractTemplateMethod
//
// Purpose-
//       Define the AbstractTemplateMethod object.
//
//----------------------------------------------------------------------------
class AbstractTemplateMethod : public Object
{
//----------------------------------------------------------------------------
// AbstractTemplateMethod::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractTemplateMethod( void );
   AbstractTemplateMethod( void );

//----------------------------------------------------------------------------
// AbstractTemplateMethod::Methods
//----------------------------------------------------------------------------
virtual void
   templateMethod( void )
{
   // :
   primitiveMethod1();
   // :
   primitiveMethod2();
   // :
}

virtual void
   primitiveMethod1( void ) = 0;

virtual void
   primitiveMethod2( void ) = 0;

//----------------------------------------------------------------------------
// AbstractTemplateMethod::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class AbstractTemplateMethod

#endif  // TEMPLATEMETHOD_H_INCLUDED
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//
// Class-
//       SampleClient
//
// Purpose-
//       Sample usage.
//
//----------------------------------------------------------------------------
class SampleClient : public Object
{
void*
   run( void )
{
   AbstractTemplateMethod*
                       templateMethod= new ConcreteTemplateMethod();

   templateMethod->templateMethoc();
} // void run
} // class SampleClient

