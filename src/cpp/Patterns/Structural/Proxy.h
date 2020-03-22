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
//       Proxy.h
//
// Purpose-
//       Provide a surrogate or placeholder for another object
//       to control access to it.
//
// Last change date-
//       2017/01/01
//
// Notes-
//       Remote proxies are responsible for encoding requests and arguments
//       and sending the encoded results to ConcreteProxies in a different
//       address space.
//
//       Virtual proxies may cache additional information about the
//       ConcreteProxy so that access to it can be postponed.  For example,
//       the loading of an image file can be deferred until it is accessed.
//
//       Protection proxies verify that a caller has access permissions
//       required to perform a request.
//
//----------------------------------------------------------------------------
#ifndef PROXY_H_INCLUDED
#define PROXY_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       AbstractProxy
//
// Purpose-
//       Define the AbstractProxy object.
//
//----------------------------------------------------------------------------
class AbstractProxy : public Object
{
//----------------------------------------------------------------------------
// AbstractProxy::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractProxy( void );

protected:
   AbstractProxy( void );

//----------------------------------------------------------------------------
// AbstractProxy::Methods
//----------------------------------------------------------------------------
public:
virtual void
   operation( void ) = 0;

//----------------------------------------------------------------------------
// AbstractProxy::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class AbstractProxy

//----------------------------------------------------------------------------
//
// Class-
//       ConcreteProxy
//
// Purpose-
//       Define the (hidden) ConcreteProxy object.
//
//----------------------------------------------------------------------------
class ConcreteProxy : public AbstractProxy
{
//----------------------------------------------------------------------------
// ConcreteProxy::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~ConcreteProxy( void );

protected:
   ConcreteProxy( void );

//----------------------------------------------------------------------------
// ConcreteProxy::Methods
//----------------------------------------------------------------------------
public:
virtual void
   operation( void );

//----------------------------------------------------------------------------
// ConcreteProxy::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class ConcreteProxy

//----------------------------------------------------------------------------
//
// Class-
//       Proxy
//
// Purpose-
//       Define the Proxy object.
//
//----------------------------------------------------------------------------
class Proxy : public AbstractProxy
{
//----------------------------------------------------------------------------
// Proxy::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Proxy( void );
   Proxy( void )
:  AbstractProxy()
,  proxy(NULL)
{
}

//----------------------------------------------------------------------------
// Proxy::Methods
//----------------------------------------------------------------------------
public:
virtual void
   operation( void )
{
   // We defer building the concrete proxy object until it's needed
   if( proxy == NULL )
     proxy= new ConcreteProxy();

   proxy->operation();
}

//----------------------------------------------------------------------------
// Proxy::Attributes
//----------------------------------------------------------------------------
   ConcreteProxy*    proxy;
}; // class Proxy
#endif  // PROXY_H_INCLUDED
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


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
   Proxy*              proxy= new Proxy();

   proxy->operation();
} // void run
} // class SampleClient

