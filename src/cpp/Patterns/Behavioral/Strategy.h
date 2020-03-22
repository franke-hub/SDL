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
//       Strategy.h
//
// Purpose-
//       Define a family of algorithms, encapsulate each one, and make them
//       interchangable.  This allows the algorigthm vary independently from
//       the clients that use it.
//
// Last change date-
//       2017/01/01
//
// Also known as-
//       Policy
//
//----------------------------------------------------------------------------
#ifndef STRATEGY_H_INCLUDED
#define STRATEGY_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class AbstractContext;
class AbstractStrategy;

//----------------------------------------------------------------------------
//
// Class-
//       AbstractStrategy
//
// Purpose-
//       Define the AbstractStrategy object.
//
//----------------------------------------------------------------------------
class AbstractStrategy : public Object
{
//----------------------------------------------------------------------------
// AbstractStrategy::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractStrategy( void );
   AbstractStrategy( void );

//----------------------------------------------------------------------------
// AbstractStrategy::Methods
//----------------------------------------------------------------------------
virtual void
   strategyInterface( void ) = 0;

//----------------------------------------------------------------------------
// AbstractStrategy::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class AbstractStrategy

//----------------------------------------------------------------------------
//
// Class-
//       AbstractContext
//
// Purpose-
//       Define the AbstractContext object.
//
//----------------------------------------------------------------------------
class AbstractContext : public Object
{
//----------------------------------------------------------------------------
// AbstractContext::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractContext( void );
   AbstractContext(
     AbstractStrategy* strategy);

//----------------------------------------------------------------------------
// AbstractContext::Methods
//----------------------------------------------------------------------------
virtual void
   contextInterface( void ) = 0;

//----------------------------------------------------------------------------
// AbstractContext::Attributes
//----------------------------------------------------------------------------
protected:
   AbstractStrategy*   strategy;
}; // class AbstractContext

#endif  // STRATEGY_H_INCLUDED
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
   AbstractContext*    context=  new ConcreteContext(new ConcreteStrategy());

   context->contextInterface();
} // void run
} // class SampleClient

