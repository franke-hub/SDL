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
//       Composite.h
//
// Purpose-
//       Decompose objects into tree structures to represent whole/part
//       hierarchies.  Clients can treat parts and components identically.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef COMPOSITE_H_INCLUDED
#define COMPOSITE_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       Component
//
// Purpose-
//       Define the Component object, the base class for Leafs and Composites.
//
//----------------------------------------------------------------------------
class Component : public Object
{
//----------------------------------------------------------------------------
// Component::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Component( void );
   Component( void );

//----------------------------------------------------------------------------
// Component::Methods
//----------------------------------------------------------------------------
public:
virtual void
   insert(                          // Insert a Component into the hierarchy
     Component*        component);  // -> Component to add

virtual void
   remove(                          // Remove a Component from the hierarchy
     Component*        component);  // -> Component to remove

virtual Component*                  // -> Component
   getComponent(                    // Get a Component
     unsigned          index);      // Using an index

virtual void
   operation( void ) = 0;           // Leaf operation

//----------------------------------------------------------------------------
// Component::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class Component

//----------------------------------------------------------------------------
//
// Class-
//       Composite
//
// Purpose-
//       Define the Composite object.
//       Its actors have different function.
//
//----------------------------------------------------------------------------
class Composite : public Component
{
//----------------------------------------------------------------------------
// Composite::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Composite( void );
   Composite( void );

//----------------------------------------------------------------------------
// Composite::Methods
//----------------------------------------------------------------------------
public:
virtual void
   insert(                          // Insert a Component into the hierarchy
     Component*        component);  // -> Component to add

virtual void
   remove(                          // Remove a Component from the hierarchy
     Component*        component);  // -> Component to remove

virtual Component*                  // -> Component
   getComponent(                    // Get a Component
     unsigned          index);      // Using an index

virtual void
   operation( void );               // Leaf operation

//----------------------------------------------------------------------------
// Composite::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class Composite

//----------------------------------------------------------------------------
//
// Class-
//       Leaf
//
// Purpose-
//       Define the Leaf object.
//
//----------------------------------------------------------------------------
class Leaf : public Component
{
//----------------------------------------------------------------------------
// Leaf::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Leaf( void );
   Leaf( void );

//----------------------------------------------------------------------------
// Leaf::Methods
//----------------------------------------------------------------------------
public:
virtual void
   operation( void );               // Leaf operation

//----------------------------------------------------------------------------
// Leaf::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class Leaf

#endif  // COMPOSITE_H_INCLUDED
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//
// Class-
//       SampleClient
//
// Purpose-
//       Use the Composite.
//
//----------------------------------------------------------------------------
class SampleClient : public Object
{
void*
   run( void )
{
   Component*             composite= new Composite();
   Component*             leaf= new Leaf();
   Component*             generic;

   composite->insert(leaf);

   for(int i=0; ; i++)
   {
     generic= composite->getComponent(i);
     if( generic == NULL )
       break;

     generic->operation();
   }
} // void run
} // class SampleClient

