//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Handler.cpp
//
// Purpose-
//       Abort, Error and Event handler.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdlib.h>

#include <com/Debug.h>
#include "com/Handler.h"

//----------------------------------------------------------------------------
//
// Method-
//       Handler::~Handler
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Handler::~Handler( void )        // Destructor
{
   #ifdef HCDM
     tracef("%8s= Handler(%p)::~Handler()\n", "", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Handler::Handler
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Handler::Handler( void )         // Constructor
:  Ident()
,  handler(NULL)
{
   #ifdef HCDM
     tracef("%8s= Handler(%p)::Handler()\n", "", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Handler::handleAbort
//
// Purpose-
//       Default abort handler.
//
//----------------------------------------------------------------------------
void
   Handler::handleAbort( void )     // Default abort handler
{
   errorf("Handler: abort(%d)\n", getIdent());
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Method-
//       Handler::handleError
//
// Purpose-
//       Default error handler.
//
//----------------------------------------------------------------------------
void
   Handler::handleError( void )     // Default error handler
{
   errorf("Handler: error(%d)\n", getIdent());
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Method-
//       Handler::handleEvent
//
// Purpose-
//       Default event handler (does nothing.)
//
//----------------------------------------------------------------------------
void
   Handler::handleEvent( void )     // Default event handler
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Handler::abort
//
// Purpose-
//       Set the abort identifier, handle the abort.
//
//----------------------------------------------------------------------------
void
   Handler::abort(                  // Indicate abort
     int               ident)       // The abort identifier
{
   Handler*            handler= this->handler; // Handler

   setIdent(ident);
   if( handler == NULL )
     handler= this;
   else
   {
     handler->setIdent(ident);
     handler->setHandler(this);
   }

   handler->handleAbort();

   // Abort must terminate
   errorf("Handler: abort(%d) exit\n", ident);
   exit(EXIT_FAILURE);              // Abort exit
}

//----------------------------------------------------------------------------
//
// Method-
//       Handler::error
//
// Purpose-
//       Set the error identifier, handle the error.
//
//----------------------------------------------------------------------------
void
   Handler::error(                  // Indicate error
     int               ident)       // The error identifier
{
   Handler*            handler= this->handler; // Handler

   setIdent(ident);
   if( handler == NULL )
     handler= this;
   else
   {
     handler->setIdent(ident);
     handler->setHandler(this);
   }

   handler->handleError();
}

//----------------------------------------------------------------------------
//
// Method-
//       Handler::event
//
// Purpose-
//       Set the event identifier, handle the event.
//
//----------------------------------------------------------------------------
void
   Handler::event(                  // Indicate event
     int               ident)       // The event identifier
{
   Handler*            handler= this->handler; // Handler

   setIdent(ident);
   if( handler == NULL )
     handler= this;
   else
   {
     handler->setIdent(ident);
     handler->setHandler(this);
   }

   handler->handleEvent();
}

