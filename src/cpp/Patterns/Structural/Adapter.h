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
//       Adapter.h
//
// Purpose-
//       Adapt the interface of one Object so it can be used as another.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef ADAPTER_H_INCLUDED
#define ADAPTER_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       AbstractAdapter
//
// Purpose-
//       Define the interface required by an Adapter.
//
//----------------------------------------------------------------------------
class AbstractAdapter : public Object
{
//----------------------------------------------------------------------------
// AbstractAdapter::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractAdapter( void ) {}
   AbstractAdapter( void ) : Object() {}

//----------------------------------------------------------------------------
// AbstractAdapter::Methods
//----------------------------------------------------------------------------
public:
virtual void
   doThat( void ) = 0;

//----------------------------------------------------------------------------
// AbstractAdapter::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class AbstractAdapter

//----------------------------------------------------------------------------
//
// Class-
//       Adaptee
//
// Purpose-
//       Object which requires an Adapter to be used as an AbstractAdaptor
//
//----------------------------------------------------------------------------
class Adaptee : public Object
{
//----------------------------------------------------------------------------
// Adaptee::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Adaptee( void ) {}
   Adaptee( void ) : Object() {}

//----------------------------------------------------------------------------
// Adaptee::Methods
//----------------------------------------------------------------------------
public:
virtual void
   doThis( void );

//----------------------------------------------------------------------------
// Adaptee::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class Adaptee

#endif  // ADAPTER_H_INCLUDED
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//
// Class-
//       SampleAdapter_Composed
//
// Purpose-
//       Sample Adapter.
//
//----------------------------------------------------------------------------
class SampleAdapter_Composed : public AbstractAdapter
{
//----------------------------------------------------------------------------
// SampleAdapter_Composed::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleAdapter_Composed( void ) {}
   SampleAdapter_Composed(
     Adaptee*          adaptee)
:  AbstractAdapter(), adaptee(adaptee) {}

//----------------------------------------------------------------------------
// SampleAdapter_Composed::Methods
//----------------------------------------------------------------------------
public:
virtual void
   doThat( void )
{
   adaptee->doThis();
}

//----------------------------------------------------------------------------
// SampleAdapter_Composed::Attributes
//----------------------------------------------------------------------------
protected:
   Adaptee*            adaptee;
}; // class SampleAdapter_Composed

//----------------------------------------------------------------------------
//
// Class-
//       SampleAdapter_Inherited
//
// Purpose-
//       Sample Adapter.
//
//----------------------------------------------------------------------------
class SampleAdapter_Inherited : public AbstractAdapter, private Adaptee
{
//----------------------------------------------------------------------------
// SampleAdapter_Inherited::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleAdapter_Inherited( void ) {}
   SampleAdapter_Inherited( void )
:  AbstractAdapter(), Adaptee() {}

//----------------------------------------------------------------------------
// SampleAdapter_Inherited::Methods
//----------------------------------------------------------------------------
public:
virtual void
   doThat( void )
{
   doThis();                        // Uses Adaptee
}

//----------------------------------------------------------------------------
// SampleAdapter_Inherited::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleAdapter_Inherited

//----------------------------------------------------------------------------
//
// Class-
//       SampleClient
//
// Purpose-
//       Use the SampleAdapter.
//
//----------------------------------------------------------------------------
class SampleClient : public Object
{
void*
   run( void )
{
   AbstractAdapter*          compose= new SampleAdapter_Composed(new Adaptee());
   AbstractAdapter*          inherit= new SampleAdapter_Inherited();

   compose->doThat();
   inherit->doThat();
} // void run
} // class SampleClient

