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
//       Bridge.h
//
// Purpose-
//       Decouple an abstraction from its implementaton so that the two
//       can vary independently.
//
// Last change date-
//       2017/01/01
//
// Also known as-
//       Handle/Body
//
//----------------------------------------------------------------------------
#ifndef BRIDGE_H_INCLUDED
#define BRIDGE_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       BridgeImplementation
//
// Purpose-
//       Define the BridgeImplementation interface, the Bridge worker object.
//
//----------------------------------------------------------------------------
class BridgeImplementation : public Object
{
//----------------------------------------------------------------------------
// BridgeImplementation::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~BridgeImplementation( void );

protected:
   BridgeImplementation( void );

//----------------------------------------------------------------------------
// BridgeImplementation::Methods
//----------------------------------------------------------------------------
public:
virtual void
   impDoA( void ) = 0;

virtual void
   impDoB( void ) = 0;

//----------------------------------------------------------------------------
// BridgeImplementation::Methods
//     Here we use the Factory pattern to get an implementation object.
//----------------------------------------------------------------------------
protected:
   friend class Bridge;
static BridgeImplementation*
   factory( void );

//----------------------------------------------------------------------------
// BridgeImplementation::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class BridgeImplementation

//----------------------------------------------------------------------------
//
// Class-
//       Bridge
//
// Purpose-
//       Define the Bridge interface, the base class for a Bridge.
//
//----------------------------------------------------------------------------
class Bridge : public Object
{
//----------------------------------------------------------------------------
// Bridge::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Bridge( void );
   Bridge( void )
{
   imp= BridgeImplementation::factory();
}

//----------------------------------------------------------------------------
// Bridge::Methods
//----------------------------------------------------------------------------
public:
virtual void
   doThat( void )
{
   imp->impDoA();
   imp->impDoB();
}

virtual void
   doThis( void )
{
   imp->impDoB();
   imp->impDoA();
}

//----------------------------------------------------------------------------
// Bridge::Attributes
//----------------------------------------------------------------------------
protected:
   BridgeImplementation*
                       imp;         // -> BridgeImplementation
}; // class Bridge

//----------------------------------------------------------------------------
//
// Class-
//       ExtendedBridge
//
// Purpose-
//       Sample ExtendedBridge.
//
//----------------------------------------------------------------------------
class ExtendedBridge : public Bridge
{
//----------------------------------------------------------------------------
// ExtendedBridge::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~ExtendedBridge( void );
   ExtendedBridge( void );

//----------------------------------------------------------------------------
// ExtendedBridge::Methods
//----------------------------------------------------------------------------
public:
virtual void
   doSomethingElse( void )
{
   doThat();
   doThis();
}

//----------------------------------------------------------------------------
// ExtendedBridge::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class ExtendedBridge

#endif  // BRIDGE_H_INCLUDED
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//
// Class-
//       SampleClient
//
// Purpose-
//       Use the Bridge.
//
//----------------------------------------------------------------------------
class SampleClient : public Object
{
void*
   run( void )
{
   Bridge*             bridge= new Bridge();
   ExtendedBridge*     extend= new ExtendedBridge();

   bridge->doThat();
   extend->doSomethingElse();
} // void run
} // class SampleClient

