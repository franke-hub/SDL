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
//       Command.h
//
// Purpose-
//       Encapsulate a request as an Object, thereby allowing clients to
//       handle different requests.  This allows queueing and logging of
//       requests and the support of undo operations.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef COMMAND_H_INCLUDED
#define COMMAND_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       AbstractCommand
//
// Purpose-
//       Define the AbstractCommand object.
//
//----------------------------------------------------------------------------
class AbstractCommand : public Object
{
//----------------------------------------------------------------------------
// AbstractCommand::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractCommand( void );

protected:
   AbstractCommand( void ) : Object(), next(NULL) {}

//----------------------------------------------------------------------------
// AbstractCommand::Methods
//----------------------------------------------------------------------------
public:
virtual void
   execute( void ) = 0;

//----------------------------------------------------------------------------
// AbstractCommand::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class AbstractCommand

//----------------------------------------------------------------------------
//
// Class-
//       Receiver
//
// Purpose-
//       Define the Receiver object, the Command handler.
//
//----------------------------------------------------------------------------
class Receiver : public Object
{
//----------------------------------------------------------------------------
// Receiver::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Receiver( void );
   Receiver( void ) : Object() {}

//----------------------------------------------------------------------------
// Receiver::Methods
//----------------------------------------------------------------------------
public:
void
   action( void );

//----------------------------------------------------------------------------
// Receiver::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class Receiver

//----------------------------------------------------------------------------
//
// Class-
//       ConcreteCommand
//
// Purpose-
//       Define a ConcreteCommand object.
//
//----------------------------------------------------------------------------
class ConcreteCommand : public AbstractCommand
{
//----------------------------------------------------------------------------
// ConcreteCommand::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~ConcreteCommand( void );
   ConcreteCommand(Receiver receiver)
   : AbstractCommand(), receiver(receiver) {}

//----------------------------------------------------------------------------
// ConcreteCommand::Methods
//----------------------------------------------------------------------------
public:
virtual void
   execute( void )
{
   receiver->action();
}

//----------------------------------------------------------------------------
// ConcreteCommand::Attributes
//----------------------------------------------------------------------------
   Receiver*           receiver;    // The target for the Command
}; // class ConcreteCommand

#endif  // COMMAND_H_INCLUDED
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
   Receiver*           receiver= new Receiver()
   Command*            command= new Command(receiver);

   command->execute();
} // void run
} // class SampleClient

