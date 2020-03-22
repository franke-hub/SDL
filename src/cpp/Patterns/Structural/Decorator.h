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
//       Decorator.h
//
// Purpose-
//       Dynamically attach additional responsibilities to particular Object
//       instances, not to an entire class of Objects.
//
// Last change date-
//       2017/01/01
//
// Also known as-
//       Wrapper.
//
//----------------------------------------------------------------------------
#ifndef DECORATOR_H_INCLUDED
#define DECORATOR_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       AbstractComponent
//
// Purpose-
//       Define the AbstractComponent object, the base class for Decorators
//       and ConcreteComponents.
//
//----------------------------------------------------------------------------
class AbstractComponent : public Object
{
//----------------------------------------------------------------------------
// AbstractComponent::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractComponent( void );
   AbstractComponent( void );

//----------------------------------------------------------------------------
// AbstractComponent::Methods
//----------------------------------------------------------------------------
public:
virtual void
   function( void ) = 0;

//----------------------------------------------------------------------------
// AbstractComponent::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class AbstractComponent

//----------------------------------------------------------------------------
//
// Class-
//       Decorator
//
// Purpose-
//       Define the (empty) Decorator object.
//
//----------------------------------------------------------------------------
class Decorator : public Component
{
//----------------------------------------------------------------------------
// Decorator::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Decorator( void );
   Decorator(
     Component*        component)
:  component(component)
{
}

//----------------------------------------------------------------------------
// Decorator::Methods
//----------------------------------------------------------------------------
public:
virtual void
   function( void )
{
   component->function();
}

//----------------------------------------------------------------------------
// Decorator::Attributes
//----------------------------------------------------------------------------
   Component*          component;
}; // class Decorator

//----------------------------------------------------------------------------
//
// Class-
//       ConcreteComponent
//
// Purpose-
//       Define the ConcreteComponent object.
//
//----------------------------------------------------------------------------
class ConcreteComponent : public Component
{
//----------------------------------------------------------------------------
// ConcreteComponent::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~ConcreteComponent( void );
   ConcreteComponent( void );

//----------------------------------------------------------------------------
// ConcreteComponent::Methods
//----------------------------------------------------------------------------
public:
virtual void
   function( void );

//----------------------------------------------------------------------------
// ConcreteComponent::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class ConcreteComponent

#endif  // DECORATOR_H_INCLUDED
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef SAMPLEDECORATOR_H_INCLUDED
#define SAMPLEDECORATOR_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       ConcreteDecoratorA
//
// Purpose-
//       Define an extended Decorator object.
//
//----------------------------------------------------------------------------
class ConcreteDecoratorA : public Decorator
{
//----------------------------------------------------------------------------
// ConcreteDecoratorA::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~ConcreteDecoratorA( void );
   ConcreteDecoratorA(
     Component*        component)
:  Decorator(component)
{
}

//----------------------------------------------------------------------------
// ConcreteDecoratorA::Methods
//----------------------------------------------------------------------------
public:
virtual void
   function( void )
{
   Decorator::function();
   ; // Added behavior
}

//----------------------------------------------------------------------------
// ConcreteDecoratorA::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class ConcreteDecoratorA

//----------------------------------------------------------------------------
//
// Class-
//       ConcreteDecoratorB
//
// Purpose-
//       Define an extended Decorator object.
//
//----------------------------------------------------------------------------
class ConcreteDecoratorB : public Decorator
{
//----------------------------------------------------------------------------
// ConcreteDecoratorB::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~ConcreteDecoratorB( void );
   ConcreteDecoratorB(
     Component*        component)
:  Decorator(component)
{
}

//----------------------------------------------------------------------------
// ConcreteDecoratorB::Methods
//----------------------------------------------------------------------------
public:
virtual void
   function( void )
{
   Decorator::function();
   ; // Added state
}

//----------------------------------------------------------------------------
// ConcreteDecoratorB::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class ConcreteDecoratorB
#endif  // SAMPLEDECORATOR_H_INCLUDED


//----------------------------------------------------------------------------
//
// Class-
//       SampleClient
//
// Purpose-
//       Use the Decorator.
//
//----------------------------------------------------------------------------
class SampleClient : public Object
{
void*
   run( void )
{
   Component*             component= new ConcreteComponent();
   Decorator*             decorator= new Decorator(component);
   Decorator*             specialA=  new ConcreteDecoratorA(decorator);
   Decorator*             specialB=  new ConcreteDecoratorB(decorator);
   Decorator*             specialC=  new ConcreteDecoratorB(specialA);

   component->function();
   decorator->function();
   speicalA->function();
   specialB->function();
   specialC->function();
} // void run
} // class SampleClient

