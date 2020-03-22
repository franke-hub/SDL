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
//       Observer.h
//
// Purpose-
//       Define a one-to-many dependency between objects so that when one
//       Object changes state, all its dependents are notified and updated.
//
// Last change date-
//       2017/01/01
//
// Also known as-
//       Dependents, Publish-Subscribe
//
//----------------------------------------------------------------------------
#ifndef OBSERVER_H_INCLUDED
#define OBSERVER_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       AbstractObserver
//
// Purpose-
//       Define the AbstractObserver object.
//
//----------------------------------------------------------------------------
class AbstractObserver : public Object
{
//----------------------------------------------------------------------------
// AbstractObserver::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractObserver( void );
   AbstractObserver( void );

//----------------------------------------------------------------------------
// AbstractObserver::Methods
//----------------------------------------------------------------------------
virtual void
   update( void ) = 0;

//----------------------------------------------------------------------------
// AbstractObserver::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class AbstractObserver

//----------------------------------------------------------------------------
//
// Class-
//       AbstractSubject
//
// Purpose-
//       Define the AbstractSubject object.
//
//----------------------------------------------------------------------------
class AbstractSubject : public Object
{
//----------------------------------------------------------------------------
// AbstractSubject::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractSubject( void );
   AbstractSubject( void );

//----------------------------------------------------------------------------
// AbstractSubject::Methods
//----------------------------------------------------------------------------
virtual void
   attach(
     Observer*         observer);

virtual void
   detach(
     Observer*         observer);

virtual void
   notify( void );

//----------------------------------------------------------------------------
// AbstractSubject::Attributes
//----------------------------------------------------------------------------
protected:                          // Sample data
   unsigned            count;       // Number of Observers
   AbstractObserver**  array;       // Observer array
}; // class AbstractSubject

//----------------------------------------------------------------------------
//
// Class-
//       ConcreteSubject
//
// Purpose-
//       Define the ConcreteSubject object.
//
//----------------------------------------------------------------------------
class ConcreteSubject : public AbstractSubject
{
//----------------------------------------------------------------------------
// ConcreteSubject::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~ConcreteSubject( void );
   ConcreteSubject( void );

//----------------------------------------------------------------------------
// ConcreteSubject::Methods
//----------------------------------------------------------------------------
virtual State&
   getState( void );

virtual void
   setState(
     State&            state);

//----------------------------------------------------------------------------
// ConcreteSubject::Attributes
//----------------------------------------------------------------------------
protected:                          // Sample data
   State               state;       // Number of Observers
}; // class ConcreteSubject

#endif  // OBSERVER_H_INCLUDED
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
   ConcreteSubject*    subject= ConcreteSubject();
   AbstractObserver*   observer1= ConcreteObserver(subject);
   AbstractObserver*   observer2= ConcreteObserver(subject);

   observer1->update();
} // void run
} // class SampleClient

