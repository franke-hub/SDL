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
//       Memento.h
//
// Purpose-
//       Capture and externalize an Object's internal state so that the
//       Object can be restored to this state later.
//
// Last change date-
//       2017/01/01
//
// Also known as-
//       Token
//
//----------------------------------------------------------------------------
#ifndef MEMENTO_H_INCLUDED
#define MEMENTO_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       AbstractMemento
//
// Purpose-
//       Define the AbstractMemento object.
//
//----------------------------------------------------------------------------
class AbstractMemento : public Object
{
//----------------------------------------------------------------------------
// AbstractMemento::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractMemento( void );
   AbstractMemento( void );

//----------------------------------------------------------------------------
// AbstractMemento::Methods
//----------------------------------------------------------------------------
   // None defined

//----------------------------------------------------------------------------
// AbstractMemento::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class AbstractMemento

//----------------------------------------------------------------------------
//
// Class-
//       Originator
//
// Purpose-
//       Define the Originator object, the Memento user.
//
//----------------------------------------------------------------------------
class Originator : public Object
{
//----------------------------------------------------------------------------
// Originator::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Originator( void );
   Originator( void );

//----------------------------------------------------------------------------
// Originator::Methods
//----------------------------------------------------------------------------
public:
AbstractMemento*
   checkpoint( void ) const;

void
   restore(
     AbstractMemento*  memento);

//----------------------------------------------------------------------------
// Originator::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class Originator

#endif  // MEMENTO_H_INCLUDED
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
   AbstractMemento*    memento;
   Originator*         originator= new Originator();

   // :
   memento= originator.checkpoint();
   // :
   if( needToRestore )
     originator.restore(memento);
   delete memento;

} // void run
} // class SampleClient

