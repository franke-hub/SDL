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
//       Prototype.h
//
// Purpose-
//       Provide an interface for duplicating an Object, but let subclasses
//       decide which physical class to instantiate.
//
// Last change date-
//       2014/01/01
//
// Also known as-
//       Cloneable
//
//----------------------------------------------------------------------------
#ifndef PROTOTYPE_H_INCLUDED
#define PROTOTYPE_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       Prototype
//
// Purpose-
//       Defines an interface for duplicating Objects which allows subclasses
//       decide which class to instantiate.
//
//----------------------------------------------------------------------------
class Prototype : public Object
{
//----------------------------------------------------------------------------
// Prototype::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// Prototype::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Prototype( void );
   Prototype( void );

//----------------------------------------------------------------------------
// Prototype::Methods
//----------------------------------------------------------------------------
public:
virtual Prototype*
   clone( void ) = 0;

//----------------------------------------------------------------------------
// Prototype::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class Prototype

#endif  // PROTOTYPE_H_INCLUDED
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//
// Title-
//       PrototypeSample.h
//
// Purpose-
//       Sample Prototype.
//
// Abstract Classes-
         class SamplePrototype;
//
// Concrete Classes-
         class SamplePrototype1;
         class SamplePrototype2;
//
// Notes-
//       Similar to AbstractCreator where the factory() method is replaced
//       by the clone() method, and there is no separate AbstractObject.
//
//----------------------------------------------------------------------------
#ifndef PROTOTYPESAMPLE_H_INCLUDED
#define PROTOTYPESAMPLE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       SamplePrototype
//
// Purpose-
//       Defines an interface for duplicating Objects which allows subclasses
//       decide which class to instantiate.
//
//----------------------------------------------------------------------------
class SamplePrototype : public Prototype
{
//----------------------------------------------------------------------------
// SamplePrototype::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SamplePrototype::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SamplePrototype( void );
   SamplePrototype( void );

//----------------------------------------------------------------------------
// SamplePrototype::Methods
//----------------------------------------------------------------------------
public:
virtual Prototype*
   clone( void ) = 0;

virtual void
   doThat( void ) = 0;

//----------------------------------------------------------------------------
// SamplePrototype::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SamplePrototype

//----------------------------------------------------------------------------
//
// Class-
//       SamplePrototype1
//
// Purpose-
//       Sample Prototype.
//
//----------------------------------------------------------------------------
class SamplePrototype1 : public SamplePrototype
{
//----------------------------------------------------------------------------
// SamplePrototype1::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SamplePrototype1::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SamplePrototype1( void );
   SamplePrototype1( void );

//----------------------------------------------------------------------------
// SamplePrototype1::Methods
//----------------------------------------------------------------------------
public:
virtual SamplePrototype*
   clone( void );

virtual void
   doThat( void );

//----------------------------------------------------------------------------
// SamplePrototype1::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SamplePrototype1

//----------------------------------------------------------------------------
//
// Class-
//       SamplePrototype2
//
// Purpose-
//       Sample Prototype.
//
//----------------------------------------------------------------------------
class SamplePrototype2 : public SamplePrototype
{
//----------------------------------------------------------------------------
// SamplePrototype2::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SamplePrototype2::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SamplePrototype2( void );
   SamplePrototype2( void );

//----------------------------------------------------------------------------
// SamplePrototype2::Methods
//----------------------------------------------------------------------------
public:
virtual SamplePrototype*
   clone( void );

virtual void
   doThat( void );

//----------------------------------------------------------------------------
// SamplePrototype2::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SamplePrototype2
#endif  // PROTOTYPESAMPLE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       PrototypeSampleClient
//
// Purpose-
//       Use the SamplePrototype.
//
//----------------------------------------------------------------------------
class PrototypeSampleClient : public Object
{
void
   run( void )
{
   SamplePrototype*    first;
   SamplePrototype*    clone;

   if( (random()%2) == 0 )
     first= new SamplePrototype1();
   else
     first= new SamplePrototype2();

   first= source->clone();
   clone->doThat();
   first->doThat();
} // void run
} // class PrototypeSampleClient

