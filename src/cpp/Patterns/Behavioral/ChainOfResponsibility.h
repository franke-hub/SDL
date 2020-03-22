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
//       ChainOfResponsibility.h
//
// Purpose-
//       Decompose objects into tree structures to represent whole/part
//       hierarchies.  Clients can treat parts and components identically.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef CHAINOFRESPONSIBILITY_H_INCLUDED
#define CHAINOFRESPONSIBILITY_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       AbstractHandler
//
// Purpose-
//       Define the AbstractHandler object.
//
//----------------------------------------------------------------------------
class AbstractHandler : public Object {
//----------------------------------------------------------------------------
// AbstractHandler::Attributes
//----------------------------------------------------------------------------
protected:
AbstractHandler*    next;           // Next AbstractHandler in chain

//----------------------------------------------------------------------------
// AbstractHandler::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractHandler( void );

protected:
   AbstractHandler( void ) : Object(), next(NULL) {}

//----------------------------------------------------------------------------
// AbstractHandler::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (negative if not handled)
   handle( void ) = 0;

#if 0
// We could also pass along a parameter specifying what is to be handled.
virtual int                         // Return code (negative if not handled)
   handle(                          // Handle a Request
     Request*          request) = 0;// -> Request
#endif
}; // class AbstractHandler

//----------------------------------------------------------------------------
//
// Class-
//       Handler
//
// Purpose-
//       Define the Handler object.
//
//----------------------------------------------------------------------------
class Handler : public AbstractHandler {
//----------------------------------------------------------------------------
// Handler::Attributes
//----------------------------------------------------------------------------
// None defined

//----------------------------------------------------------------------------
// Handler::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Handler( void ) {}
   Handler( void ) : AbstractHandler() {}

//----------------------------------------------------------------------------
// Handler::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (negative if not handled)
   handle( void )
{
   for(AbstractHandler* h= next; h != NULL; h= h->next)
   {
     int rc= h->handle();
     if( rc >= 0 )
       return rc;
   }

   throw "NoHandlerException";
}

virtual void
   insert(                          // Insert new AbstractHandler onto list
     AbstractHandler*  hand)        // -> AbstractHandler
{
   hand->next= next;
   next= hand;
}
}; // class Handler

//----------------------------------------------------------------------------
//
// Class-
//       ConcreteHandler1
//
// Purpose-
//       Sample handler.
//
//----------------------------------------------------------------------------
class ConcreteHandler1 : public AbstractHandler {
//----------------------------------------------------------------------------
// ConcreteHandler1::Attributes
//----------------------------------------------------------------------------
// None defined

//----------------------------------------------------------------------------
// ConcreteHandler1::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~ConcreteHandler1( void );
   ConcreteHandler1( void ) : AbstractHandler() {}

//----------------------------------------------------------------------------
// ConcreteHandler1::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (negative if not handled)
   handle( void )
{
   return 0;
}
}; // class ConcreteHandler1

//----------------------------------------------------------------------------
//
// Class-
//       ConcreteHandler2
//
// Purpose-
//       Sample handler.
//
//----------------------------------------------------------------------------
class ConcreteHandler2 : public AbstractHandler {
//----------------------------------------------------------------------------
// ConcreteHandler2::Attributes
//----------------------------------------------------------------------------
// None defined

//----------------------------------------------------------------------------
// ConcreteHandler2::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~ConcreteHandler2( void );
   ConcreteHandler2( void ) : AbstractHandler() {}

//----------------------------------------------------------------------------
// ConcreteHandler2::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (negative if not handled)
   handle( void )
{
   return -1;
}
}; // class ConcreteHandler2

#endif  // CHAINOFRESPONSIBILITY_H_INCLUDED
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
class SampleClient : public Object {
void*
   run( void )
{
   Handler*            handler= new Handler();

   handler->insert(new ConcreteHandler1());
   handler->insert(new ConcreteHandler2());

   handler->handle();
} // void run
}; // class SampleClient

