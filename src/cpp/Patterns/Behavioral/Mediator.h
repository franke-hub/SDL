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
//       Moderator.h
//
// Purpose-
//       Define an object that encapsulates how a set of Objects interact.
//       Mediator promotes loose coupling by keeping Objects from referring
//       to each other explicitly and allows you to vary their interactions
//       independently.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef MODERATOR_H_INCLUDED
#define MODERATOR_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       AbstractModerator
//
// Purpose-
//       Define the AbstractModerator object.
//       An abstract class is required only when the Colleagues can interact
//       with various concrete Moderators.  If there is only one Moderator,
//       there is no need for an abstract base class.
//
//----------------------------------------------------------------------------
class AbstractModerator : public Object
{
//----------------------------------------------------------------------------
// AbstractModerator::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractModerator( void );

protected:
   AbstractModerator( void ):

//----------------------------------------------------------------------------
// AbstractModerator::Methods
//----------------------------------------------------------------------------
virtual void
   mediate( void ) = 0;

//----------------------------------------------------------------------------
// AbstractModerator::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class AbstractModerator

//----------------------------------------------------------------------------
//
// Class-
//       AbstractColleague
//
// Purpose-
//       Define the AbstractColleague object.
//
//----------------------------------------------------------------------------
class AbstractColleague : public Object
{
//----------------------------------------------------------------------------
// AbstractColleague::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractColleague( void );

protected:
   AbstractColleague(
     Mediator*         mediator);

//----------------------------------------------------------------------------
// AbstractColleague::Methods
//----------------------------------------------------------------------------
   // None defined

//----------------------------------------------------------------------------
// AbstractColleague::Attributes
//----------------------------------------------------------------------------
protected:
   Mediator*           mediator;
}; // class AbstractColleague

#endif  // MODERATOR_H_INCLUDED
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
   AbstractMediator*   mediator= ConcreteMediator();
   AbstractColleague*  colleague1= ConcreteColleague1(mediator);
   AbstractColleague*  colleague2= ConcreteColleague2(mediator);

   colleague1->doThis();            // Interacts with colleague2
   colleague2->doThat();            // Interacts with colleague1

} // void run
} // class SampleClient

