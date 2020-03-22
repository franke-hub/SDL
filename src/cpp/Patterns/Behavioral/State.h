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
//       State.h
//
// Purpose-
//       Allow an object to alter its behavior when its internal state changes.
//       This object will appear to change its class.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class AbstractContext;
class AbstractState;

//----------------------------------------------------------------------------
//
// Class-
//       AbstractState
//
// Purpose-
//       Define the AbstractState object.
//
//----------------------------------------------------------------------------
class AbstractState : public Object
{
//----------------------------------------------------------------------------
// AbstractState::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractState( void );
   AbstractState( void );

//----------------------------------------------------------------------------
// AbstractState::Methods
//----------------------------------------------------------------------------
virtual void
   handle(
     AbtractContext*   context) = 0;

//----------------------------------------------------------------------------
// AbstractState::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class AbstractState

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
   AbstractContext( void );

//----------------------------------------------------------------------------
// AbstractContext::Methods
//----------------------------------------------------------------------------
virtual void
   handle( void ) = 0;

//----------------------------------------------------------------------------
// AbstractContext::Attributes
//----------------------------------------------------------------------------
protected:
   AbtractState*       state;
}; // class AbstractContext

#endif  // STATE_H_INCLUDED
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
   ConcreteContext*    context= new ConcreteContext();

   context->handle();               // May create or change state
   context->handle();
   context->handle();
} // void run
} // class SampleClient

