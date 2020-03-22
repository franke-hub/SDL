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
//       AbstractFactory.h
//
// Purpose-
//       Provide an interface for creating families of related or dependent
//       Objects without specifying their concrete class.
//
// Last change date-
//       2014/01/01
//
// Also known as-
//       Kit
//
//----------------------------------------------------------------------------
#ifndef ABSTRACTFACTORY_H_INCLUDED
#define ABSTRACTFACTORY_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       AbstractFactory
//
// Purpose-
//       Defines an interface for creating families of related or dependent
//       Objects without specifying their concrete class.
//
//----------------------------------------------------------------------------
class AbstractFactory : public Object
{
//----------------------------------------------------------------------------
// AbstractFactory::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// AbstractFactory::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractFactory( void );
   AbstractFactory( void );

//----------------------------------------------------------------------------
// AbstractFactory::Methods
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// AbstractFactory::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class AbstractFactory

#endif  // ABSTRACTFACTORY_H_INCLUDED
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//
// Title-
//       AbstractFactorySample.h
//
// Purpose-
//       Sample AbstractFactory.
//
// Abstract Classes-
         class SampleAbstractObject1;
         class SampleAbstractObject2;

         class SampleAbstractFactory;

// Concrete Classes-
         class SampleFactory1ConcreteObject1;
         class SampleFactory1ConcreteObject2;
         class SampleConcreteFactory1;

         class SampleFactory2ConcreteObject1;
         class SampleFactory2ConcreteObject2;
         class SampleConcreteFactory2;
//
//----------------------------------------------------------------------------
#ifndef ABSTRACTFACTORYSAMPLE_H_INCLUDED
#define ABSTRACTFACTORYSAMPLE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       SampleAbstractObject1
//
// Purpose-
//       Sample AbstractObject.
//
//----------------------------------------------------------------------------
class SampleAbstractObject1 : public Object
{
//----------------------------------------------------------------------------
// SampleAbstractObject1::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleAbstractObject1::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleAbstractObject1( void );
   SampleAbstractObject1( void );

//----------------------------------------------------------------------------
// SampleAbstractObject1::Methods
//----------------------------------------------------------------------------
public:
virtual void
   doThis( void ) = 0;

//----------------------------------------------------------------------------
// SampleAbstractObject1::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleAbstractObject1

//----------------------------------------------------------------------------
//
// Class-
//       SampleAbstractObject2
//
// Purpose-
//       Sample AbstractObject.
//
//----------------------------------------------------------------------------
class SampleAbstractObject2 : public Object
{
//----------------------------------------------------------------------------
// SampleAbstractObject2::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleAbstractObject2::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleAbstractObject2( void );
   SampleAbstractObject2( void );

//----------------------------------------------------------------------------
// SampleAbstractObject2::Methods
//----------------------------------------------------------------------------
public:
virtual void
   doThat( void ) = 0;

//----------------------------------------------------------------------------
// SampleAbstractObject2::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleAbstractObject2

//----------------------------------------------------------------------------
//
// Class-
//       SampleAbstractFactory
//
// Purpose-
//       Sample AbstractFactory.
//
//----------------------------------------------------------------------------
class SampleAbstractFactory : public AbstractFactory
{
//----------------------------------------------------------------------------
// SampleAbstractFactory::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleAbstractFactory::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleAbstractFactory( void );
   SampleAbstractFactory( void );

//----------------------------------------------------------------------------
// SampleAbstractFactory::Methods
//----------------------------------------------------------------------------
public:
virtual SampleAbstractObject1*
   createObject1( void ) = 0;

virtual SampleAbstractObject2*
   createObject2( void ) = 0;

//----------------------------------------------------------------------------
// SampleAbstractFactory::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleAbstractFactory

//----------------------------------------------------------------------------
//
// Class-
//       SampleFactory1ConcreteObject1
//
// Purpose-
//       Sample Factory1ConcreteObject.
//
//----------------------------------------------------------------------------
class SampleFactory1ConcreteObject1 : public AbstractObject1
{
//----------------------------------------------------------------------------
// SampleFactory1ConcreteObject1::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleFactory1ConcreteObject1::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleFactory1ConcreteObject1( void );
   SampleFactory1ConcreteObject1( void );

//----------------------------------------------------------------------------
// SampleFactory1ConcreteObject1::Methods
//----------------------------------------------------------------------------
public:
virtual void
   doThis( void );

//----------------------------------------------------------------------------
// SampleFactory1ConcreteObject1::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleFactory1ConcreteObject1

//----------------------------------------------------------------------------
//
// Class-
//       SampleFactory1ConcreteObject2
//
// Purpose-
//       Sample Factory1ConcreteObject.
//
//----------------------------------------------------------------------------
class SampleFactory1ConcreteObject2 : public AbstractObject2
{
//----------------------------------------------------------------------------
// SampleFactory1ConcreteObject2::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleFactory1ConcreteObject2::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleFactory1ConcreteObject2( void );
   SampleFactory1ConcreteObject2( void );

//----------------------------------------------------------------------------
// SampleFactory1ConcreteObject2::Methods
//----------------------------------------------------------------------------
public:
virtual void
   doThat( void );

//----------------------------------------------------------------------------
// SampleFactory1ConcreteObject2::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleFactory1ConcreteObject2

//----------------------------------------------------------------------------
//
// Class-
//       SampleConcreteFactory1
//
// Purpose-
//       Sample ConcreteFactory.
//
//----------------------------------------------------------------------------
class SampleConcreteFactory1 : public SampleAbstractFactory
{
//----------------------------------------------------------------------------
// SampleConcreteFactory1::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleConcreteFactory1::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleConcreteFactory1( void );
   SampleConcreteFactory1( void );

//----------------------------------------------------------------------------
// SampleConcreteFactory1::Methods
//----------------------------------------------------------------------------
public:
virtual SampleAbstractObject1*
   createObject1( void );

virtual SampleAbstractObject2*
   createObject2( void );

//----------------------------------------------------------------------------
// SampleConcreteFactory1::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleConcreteFactory1

//----------------------------------------------------------------------------
//
// Class-
//       SampleFactory2ConcreteObject1
//
// Purpose-
//       Sample Factory2ConcreteObject.
//
//----------------------------------------------------------------------------
class SampleFactory2ConcreteObject1 : public AbstractObject1
{
//----------------------------------------------------------------------------
// SampleFactory2ConcreteObject1::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleFactory2ConcreteObject1::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleFactory2ConcreteObject1( void );
   SampleFactory2ConcreteObject1( void );

//----------------------------------------------------------------------------
// SampleFactory2ConcreteObject1::Methods
//----------------------------------------------------------------------------
public:
virtual void
   doThis( void );

//----------------------------------------------------------------------------
// SampleFactory2ConcreteObject1::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleFactory2ConcreteObject1

//----------------------------------------------------------------------------
//
// Class-
//       SampleFactory2ConcreteObject2
//
// Purpose-
//       Sample Factory2ConcreteObject.
//
//----------------------------------------------------------------------------
class SampleFactory2ConcreteObject2 : public AbstractObject2
{
//----------------------------------------------------------------------------
// SampleFactory2ConcreteObject2::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleFactory2ConcreteObject2::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleFactory2ConcreteObject2( void );
   SampleFactory2ConcreteObject2( void );

//----------------------------------------------------------------------------
// SampleFactory2ConcreteObject2::Methods
//----------------------------------------------------------------------------
public:
virtual void
   doThat( void );

//----------------------------------------------------------------------------
// SampleFactory2ConcreteObject2::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleFactory2ConcreteObject2

//----------------------------------------------------------------------------
//
// Class-
//       SampleConcreteFactory2
//
// Purpose-
//       Sample ConcreteFactory.
//
//----------------------------------------------------------------------------
class SampleConcreteFactory2 : public SampleAbstractFactory
{
//----------------------------------------------------------------------------
// SampleConcreteFactory2::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleConcreteFactory2::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleConcreteFactory2( void );
   SampleConcreteFactory2( void );

//----------------------------------------------------------------------------
// SampleConcreteFactory2::Methods
//----------------------------------------------------------------------------
public:
virtual SampleAbstractObject1*
   createObject1( void );

virtual SampleAbstractObject2*
   createObject2( void );

//----------------------------------------------------------------------------
// SampleConcreteFactory2::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleConcreteFactory2
#endif  // ABSTRACTFACTORYSAMPLE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       AbstractFactorySampleClient
//
// Purpose-
//       Use the SampleAbstractFactory.
//       The implementation is selected only once, when the factory is built.
//       The implementation can be selected at runtime.
//
//----------------------------------------------------------------------------
class AbstractFactorySampleClient : public Object
{
void
   run( void )
{
   SampleAbstractFactory*    factory;
   SampleAbstractObject1*    object1;
   SampleAbstractObject2*    object2;

   if( (random()%2) == 0 )
     factory= new SampleConcreteFactory1();
   else
     factory= new SampleConcreteFactory2();

   object1= factory->createObject1();
   object2= factory->createObject2();

   object1->doThis();
   object2->doThat();
} // void run
} // class AbstractFactorySampleClient

