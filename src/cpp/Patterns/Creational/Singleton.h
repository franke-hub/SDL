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
//       Singleton.h
//
// Purpose-
//       Ensure that a class has only one instance and provide a global
//       access point for it.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef SINGLETON_H_INCLUDED
#define SINGLETON_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       Singleton
//
// Purpose-
//       Defines a class which has only one instance and provides a global
//       access point for it.
//
//----------------------------------------------------------------------------
class Singleton : public Object {
//----------------------------------------------------------------------------
// Singleton::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
// None defined

//----------------------------------------------------------------------------
// Singleton::Attributes
//----------------------------------------------------------------------------
protected:
// None defined

//----------------------------------------------------------------------------
// Singleton::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Singleton( void );

protected:
   Singleton( void );

//----------------------------------------------------------------------------
// Singleton::Methods
//----------------------------------------------------------------------------
public:
static Singleton&
   instance( void );

//----------------------------------------------------------------------------
// Singleton::Methods used to install a subclass of Singleton
//----------------------------------------------------------------------------
static void
   install(                         // OPTIONAL: Install the Singleton
     const char*     name);

protected:
static void
   insert(                          // OPTIONAL: Add Singleton to registry
     const char*     name,
     Singleton*      value);

static Singleton*
   locate(                          // OPTIONAL: Locate Singleton subclass
     const char*     name);
}; // class Singleton

#endif  // SINGLETON_H_INCLUDED
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//
// Title-
//       SingletonSample.h
//
// Purpose-
//       Sample Singleton.
//
// Abstract Classes-
         class SampleSingleton;
//
// Concrete Classes-
         class SampleSingleton0;
         class SampleSingleton1;
//
//----------------------------------------------------------------------------
#ifndef SINGLETONSAMPLE_H_INCLUDED
#define SINGLETONSAMPLE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       SampleSingleton
//
// Purpose-
//       Defines an interface for duplicating Objects which allows subclasses
//       decide which class to instantiate.
//
//----------------------------------------------------------------------------
class SampleSingleton : public Object
{
//----------------------------------------------------------------------------
// SampleSingleton::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleSingleton::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleSingleton( void );

protected:
   SampleSingleton(
     const char*       name,
     SampleSingleton*  value)
:  Object()
{
   install(name,value);
}

//----------------------------------------------------------------------------
// SampleSingleton::Methods
//----------------------------------------------------------------------------
public:
virtual const char*
   doThat( void ) = 0;

static SampleSingleton*
   instance( void );
{
   const char*         name;

   if( _instance == NULL )
   {
     _barrier.raise();
     if( _instance == NULL )
     {
       name= getenv("SINGLETON");
       if( name == NULL )
         name= "DefaultSingleton";

       _instance= hashTable.get(name);
     }
     _barrier.lower();
   }

   return _instance;
}

static void
   install(                         // OPTIONAL: Install the Singleton
     const char*     name)
{
   Singleton*        target;

   target= locate(name);
   if( _instance == NULL )
   {
     _barrier.raise();
     if( _instance == NULL )
       _instance= target;
     _barrier.lower();
   }

   if( target == NULL || target != instance )
     throw new Exception("Singleton.install(%s): Unable to comply", name);
}

protected:
static void
   insert(                          // OPTIONAL: Add Singleton to registry
     const char*     name,
     Singleton*      value)
{
   _barrier.raise();
   hashTable.put(name, value);
   _barrier.lower();
}

static Singleton*
   locate(                          // OPTIONAL: Locate Singleton subclass
     const char*     name);
{
   _barrier.raise();
   _instance= hashTable.get(name);
   _barrier.lower();
}

//----------------------------------------------------------------------------
// SampleSingleton::Attributes
//----------------------------------------------------------------------------
protected:
static Barrier         _barrier;
static Singleton*      _instance;
static HashTable       hashTable;   // Name-value pair
}; // class SampleSingleton

//----------------------------------------------------------------------------
//
// Class-
//       SampleSingleton0
//
// Purpose-
//       Sample Singleton.
//
//----------------------------------------------------------------------------
class SampleSingleton0 : public SampleSingleton
{
//----------------------------------------------------------------------------
// SampleSingleton0::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleSingleton0::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleSingleton0( void );
   SampleSingleton0( void )
:  SampleSingleton("DefaultSingleton", this) {}

//----------------------------------------------------------------------------
// SampleSingleton0::Methods
//----------------------------------------------------------------------------
public:
virtual const char*
   doThat( void );
{
   return "DefaultSingleton";
}

//----------------------------------------------------------------------------
// SampleSingleton0::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleSingleton0

static SampleSingleton0  __SampleSingleton0; // Instantiate, add to registry

//----------------------------------------------------------------------------
//
// Class-
//       SampleSingleton1
//
// Purpose-
//       Sample Singleton.
//
//----------------------------------------------------------------------------
class SampleSingleton1 : public SampleSingleton
{
//----------------------------------------------------------------------------
// SampleSingleton1::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
   // None defined

//----------------------------------------------------------------------------
// SampleSingleton1::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SampleSingleton1( void );
   SampleSingleton1( void )
:  SampleSingleton("MySingleton", this) {}

//----------------------------------------------------------------------------
// SampleSingleton1::Methods
//----------------------------------------------------------------------------
public:
virtual const char*
   doThat( void )
{
   return "MySingleton";
}

//----------------------------------------------------------------------------
// SampleSingleton1::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class SampleSingleton1
#endif  // SINGLETONSAMPLE_H_INCLUDED

static SampleSingleton1  __SampleSingleton1; // Instantiate, add to registry

//----------------------------------------------------------------------------
//
// Class-
//       SingletonSampleClient
//
// Purpose-
//       Use the SampleSingleton.
//
//----------------------------------------------------------------------------
class SingletonSampleClient : public Object
{
void
   run( void )
{
   if( SampleSingleton::instance() == NULL )
     SampleSingleton::install("MySingleton");

   printf("Got: %s\n", SampleSingleton::instance()->doThat());
} // void run
}; // class SingletonSampleClient

