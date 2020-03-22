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
//       Handler.h
//
// Purpose-
//       Abort, Error and Event handler
//
// Last change date-
//       2007/01/01
//
// Classes-
         class Handler;             // Abort, Error and Event handler
//
//----------------------------------------------------------------------------
#ifndef HANDLER_H_INCLUDED
#define HANDLER_H_INCLUDED

#ifndef IDENT_H_INCLUDED
#include "Ident.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Handler
//
// Purpose-
//       Event, error and abort handler.
//
//
// event(), error() or abort() logic description-
//       setIdent(code)
//       if( handler == NULL )
//         handleXXXXX()
//       else {
//         handler->setIdent(code)
//         handler->setHandler(this)
//         handler->handleXXXXX()
//       }
//
//----------------------------------------------------------------------------
class Handler : public Ident {      // Abort, Error and Event handler
//----------------------------------------------------------------------------
// Handler::Attributes
//----------------------------------------------------------------------------
private:
Handler*               handler;     // -> Associated Handler/Object

//----------------------------------------------------------------------------
// Handler::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Handler( void );                // Destructor
   Handler( void );                 // Constructor

//----------------------------------------------------------------------------
// Handler::Methods (normally not overridden)
//----------------------------------------------------------------------------
public:
inline Handler*                     // -> Handler
   getHandler( void );              // Extract the Handler

inline void
   setHandler(                      // Set the Handler
     Handler*          handler);    // -> Handler

void                                // DOES NOT RETURN
   abort(                           // Indicate abort condition
     int               ident);      // The abort identifier

void
   error(                           // Indicate error condition
     int               ident);      // The error identifier

void
   event(                           // Indicate event condition
     int               ident);      // The event identifier

//----------------------------------------------------------------------------
// Handler::Virtual Methods, normally supplied in derived class
//----------------------------------------------------------------------------
protected:
virtual void
   handleAbort( void );             // Abort handler

virtual void
   handleError( void );             // Error handler

virtual void
   handleEvent( void );             // Event handler
}; // class Handler

#include "Handler.i"

#endif // HANDLER_H_INCLUDED
