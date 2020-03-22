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
//       AbstractBuilder.h
//
// Purpose-
//       Separate the constructon of a complex object from its representation
//       so that the same construction process can create different
//       representations.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef ABSTRACTBUILDER_H_INCLUDED
#define ABSTRACTBUILDER_H_INCLUDED

#ifndef OBJECT_H_INCLUDED
#include ".Object/Object.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       AbstractBuilder
//
// Purpose-
//       Sample AbstractBuilder.
//
//----------------------------------------------------------------------------
class AbstractBuilder : public Object
{
//----------------------------------------------------------------------------
// AbstractBuilder::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// AbstractBuilder::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractBuilder( void );
   AbstractBuilder( void );

//----------------------------------------------------------------------------
// AbstractBuilder::Methods
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// AbstractBuilder::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class AbstractBuilder

//----------------------------------------------------------------------------
//
// Class-
//       AbstractDirector
//
// Purpose-
//       Direct the construction of an AbstractBuilder
//
//----------------------------------------------------------------------------
class AbstractDirector : public Object
{
//----------------------------------------------------------------------------
// AbstractDirector::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// AbstractDirector::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractDirector( void );
   AbstractDirector(
     AbstractBuilder*  builder);

//----------------------------------------------------------------------------
// AbstractDirector::Methods
//----------------------------------------------------------------------------
public:
virtual void
   build( void );                   // Build the object

//----------------------------------------------------------------------------
// AbstractDirector::Attributes
//----------------------------------------------------------------------------
protected:
   AbstractBuilder*    builder;
}; // class AbstractDirector

#endif  // ABSTRACTBUILDER_H_INCLUDED
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//
// Title-
//       AbstractBuilderSample.h
//
// Purpose-
//       Sample AbstractBuilder.
//
// Abstract Classes-
         class SampleAbstractObject1;
         class SampleAbstractObject2;

         class SampleAbstractBuilder;

// Concrete Classes-
         class SampleBuilder1ConcreteObject1;
         class SampleBuilder1ConcreteObject2;
         class SampleConcreteBuilder1;

         class SampleBuilder2ConcreteObject1;
         class SampleBuilder2ConcreteObject2;
         class SampleConcreteBuilder2;
//
//----------------------------------------------------------------------------
#ifndef ABSTRACTBUILDERSAMPLE_H_INCLUDED
#define ABSTRACTBUILDERSAMPLE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       SampleAbstractBuilder
//
// Purpose-
//       Sample AbstractBuilder.
//
//----------------------------------------------------------------------------
class SampleAbstractBuilder : public AbstractBuilder
{
//----------------------------------------------------------------------------
// SampleAbstractBuilder::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleAbstractBuilder::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleAbstractBuilder( void );
   SampleAbstractBuilder( void );

//----------------------------------------------------------------------------
// SampleAbstractBuilder::Methods
//----------------------------------------------------------------------------
public:
virtual void
   buildPartA( void ) = 0;

virtual void
   buildPartB( void ) = 0;

virtual void
   buildPartC( void ) = 0;

//----------------------------------------------------------------------------
// SampleAbstractBuilder::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleAbstractBuilder

//----------------------------------------------------------------------------
//
// Class-
//       SampleConcreteBuilder1
//
// Purpose-
//       Sample ConcreteBuilder.
//
//----------------------------------------------------------------------------
class SampleConcreteBuilder1 : public SampleAbstractBuilder
{
//----------------------------------------------------------------------------
// SampleConcreteBuilder1::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleConcreteBuilder1::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleConcreteBuilder1( void );
   SampleConcreteBuilder1( void );

//----------------------------------------------------------------------------
// SampleConcreteBuilder1::Methods
//----------------------------------------------------------------------------
public:
virtual void
   buildPartA( void );

virtual void
   buildPartB( void );

virtual void
   buildPartC( void );

void
   buildPartX( void );

void*
   resultant( void );

//----------------------------------------------------------------------------
// SampleConcreteBuilder1::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleConcreteBuilder1

//----------------------------------------------------------------------------
//
// Class-
//       SampleConcreteBuilder2
//
// Purpose-
//       Sample (unused) ConcreteBuilder.
//
//----------------------------------------------------------------------------
class SampleConcreteBuilder2 : public SampleAbstractBuilder
{
//----------------------------------------------------------------------------
// SampleConcreteBuilder2::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleConcreteBuilder2::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleConcreteBuilder2( void );
   SampleConcreteBuilder2( void );

//----------------------------------------------------------------------------
// SampleConcreteBuilder2::Methods
//----------------------------------------------------------------------------
public:
virtual void
   buildPartA( void );

virtual void
   buildPartB( void );

virtual void
   buildPartC( void );

void
   buildPartY( void );

void*
   resultant( void );

//----------------------------------------------------------------------------
// SampleConcreteBuilder2::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleConcreteBuilder2
#endif  // ABSTRACTBUILDERSAMPLE_H_INCLUDED


//----------------------------------------------------------------------------
//
// Class-
//       SampleAbstractBuilderSampleClient
//
// Purpose-
//       Use the SampleAbstractBuilder.
//       The director does not know or care about Builder internals.
//
//----------------------------------------------------------------------------
class AbstractBuilderSampleClient : public Object
{
void*
   run( void )
{
   SampleConcreteBuilder1*   builder1= new SampleConcreteBuilder1();
   SampleAbstractDirector*   director= new SampleAbtractDirector(builder1);

   director->build();               // Build without knowledge of builder
   builder1->buildPartX();          // Specific to SampleConcreteBuilder1
   return builder1->resultant();    // Specific to SampleConcreteBuilder1
} // void run
} // class AbstractBuilderSampleClient

