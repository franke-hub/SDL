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
//       AbstractCreator.h
//
// Purpose-
//       Provide an interface for creating an Object, but let subclasses
//       decide which class to instantiate.
//
// Last change date-
//       2014/01/01
//
// Also known as-
//       FactoryMethod
//       VirtualConstructor
//
//----------------------------------------------------------------------------
#ifndef ABSTRACTCREATOR_H_INCLUDED
#define ABSTRACTCREATOR_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       AbstractObject
//
// Purpose-
//       Defines the Object built by our AbstractCreator.
//
//----------------------------------------------------------------------------
class AbstractObject : public Object
{
//----------------------------------------------------------------------------
// AbstractObject::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// AbstractObject::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractObject( void );
   AbstractObject( void );

//----------------------------------------------------------------------------
// AbstractObject::Methods
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// AbstractObject::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class AbstractObject

//----------------------------------------------------------------------------
//
// Class-
//       AbstractCreator
//
// Purpose-
//       Defines an interface for creating Objects which allows subclasses
//       decide which class to instantiate.
//
//----------------------------------------------------------------------------
class AbstractCreator : public Object
{
//----------------------------------------------------------------------------
// AbstractCreator::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// AbstractCreator::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractCreator( void );
   AbstractCreator( void );

//----------------------------------------------------------------------------
// AbstractCreator::Methods
//----------------------------------------------------------------------------
public:
virtual AbstractObject*
   factory( void );

//----------------------------------------------------------------------------
// AbstractCreator::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class AbstractCreator

#endif  // ABSTRACTCREATOR_H_INCLUDED
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//
// Title-
//       AbstractCreatorSample.h
//
// Purpose-
//       Sample AbstractCreator.
//
// Abstract Classes-
         class SampleAbstractObject;
         class SampleAbstractCreator;

// Concrete Classes-
         class SampleCreator1ConcreteObject;
         class SampleConcreteCreator1;

         class SampleCreator2ConcreteObject;
         class SampleConcreteCreator2;
//
// Notes-
//       Very similar to AbstractFactorySample.h
//
//----------------------------------------------------------------------------
#ifndef ABSTRACTCREATORSAMPLE_H_INCLUDED
#define ABSTRACTCREATORSAMPLE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       SampleAbstractObject
//
// Purpose-
//       Sample AbstractObject.
//
//----------------------------------------------------------------------------
class SampleAbstractObject : public AbstractObject
{
//----------------------------------------------------------------------------
// SampleAbstractObject::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleAbstractObject::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleAbstractObject( void );
   SampleAbstractObject( void );

//----------------------------------------------------------------------------
// SampleAbstractObject::Methods
//----------------------------------------------------------------------------
public:
virtual void
   doThat( void ) = 0;

//----------------------------------------------------------------------------
// SampleAbstractObject::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleAbstractObject

//----------------------------------------------------------------------------
//
// Class-
//       SampleAbstractCreator
//
// Purpose-
//       Sample AbstractCreator.
//
//----------------------------------------------------------------------------
class SampleAbstractCreator : public AbstractCreator
{
//----------------------------------------------------------------------------
// SampleAbstractCreator::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleAbstractCreator::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleAbstractCreator( void );
   SampleAbstractCreator( void );

//----------------------------------------------------------------------------
// SampleAbstractCreator::Methods
//----------------------------------------------------------------------------
public:
virtual SampleAbstractObject*
   factory( void ) = 0;

//----------------------------------------------------------------------------
// SampleAbstractCreator::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleAbstractCreator

//----------------------------------------------------------------------------
//
// Class-
//       SampleCreator1ConcreteObject
//
// Purpose-
//       Sample Creator1ConcreteObject.
//
//----------------------------------------------------------------------------
class SampleCreator1ConcreteObject : public SampleAbstractObject
{
//----------------------------------------------------------------------------
// SampleCreator1ConcreteObject::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleCreator1ConcreteObject::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleCreator1ConcreteObject( void );
   SampleCreator1ConcreteObject( void );

//----------------------------------------------------------------------------
// SampleCreator1ConcreteObject::Methods
//----------------------------------------------------------------------------
public:
virtual void
   doThat( void );

//----------------------------------------------------------------------------
// SampleCreator1ConcreteObject::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleCreator1ConcreteObject

//----------------------------------------------------------------------------
//
// Class-
//       SampleConcreteCreator1
//
// Purpose-
//       Sample ConcreteCreator.
//
//----------------------------------------------------------------------------
class SampleConcreteCreator1 : public SampleAbstractCreator
{
//----------------------------------------------------------------------------
// SampleConcreteCreator1::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleConcreteCreator1::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleConcreteCreator1( void );
   SampleConcreteCreator1( void );

//----------------------------------------------------------------------------
// SampleConcreteCreator1::Methods
//----------------------------------------------------------------------------
public:
virtual SampleAbstractObject*
   factory( void );

//----------------------------------------------------------------------------
// SampleConcreteCreator1::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleConcreteCreator1

//----------------------------------------------------------------------------
//
// Class-
//       SampleCreator2ConcreteObject
//
// Purpose-
//       Sample Creator2ConcreteObject.
//
//----------------------------------------------------------------------------
class SampleCreator2ConcreteObject : public SampleAbstractObject
{
//----------------------------------------------------------------------------
// SampleCreator2ConcreteObject::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleCreator2ConcreteObject::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleCreator2ConcreteObject( void );
   SampleCreator2ConcreteObject( void );

//----------------------------------------------------------------------------
// SampleCreator2ConcreteObject::Methods
//----------------------------------------------------------------------------
public:
virtual void
   doThat( void );

//----------------------------------------------------------------------------
// SampleCreator2ConcreteObject::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleCreator2ConcreteObject

//----------------------------------------------------------------------------
//
// Class-
//       SampleConcreteCreator2
//
// Purpose-
//       Sample ConcreteCreator.
//
//----------------------------------------------------------------------------
class SampleConcreteCreator2 : public SampleAbstractCreator
{
//----------------------------------------------------------------------------
// SampleConcreteCreator2::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleConcreteCreator2::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleConcreteCreator2( void );
   SampleConcreteCreator2( void );

//----------------------------------------------------------------------------
// SampleConcreteCreator2::Methods
//----------------------------------------------------------------------------
public:
virtual SampleAbstractObject*
   factory( void );

//----------------------------------------------------------------------------
// SampleConcreteCreator2::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleConcreteCreator2
#endif  // ABSTRACTCREATORSAMPLE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       AbstractCreatorSampleClient
//
// Purpose-
//       Use the SampleAbstractCreator.
//       The implementation is selected only once, when the Creator is built.
//       The implementation can be selected at runtime.
//
//----------------------------------------------------------------------------
class AbstractCreatorSampleClient : public Object
{
void
   run( void )
{
   SampleAbstractCreator*    Creator;
   SampleAbstractObject*     object;

   if( (random()%2) == 0 )
     Creator= new SampleConcreteCreator1();
   else
     Creator= new SampleConcreteCreator2();

   object= Creator->factory();
   object->doThat();
} // void run
} // class AbstractCreatorSampleClient

