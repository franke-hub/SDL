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
//       Handler.i
//
// Purpose-
//       Handler inline methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef HANDLER_I_INCLUDED
#define HANDLER_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       InternalHandler::getHandler
//
// Purpose-
//       Get the Handler.
//
//----------------------------------------------------------------------------
Handler*                            // The Handler
   Handler::getHandler( void )      // Extract the Handler
{
   return handler;
}

//----------------------------------------------------------------------------
//
// Method-
//       Handler::setHandler
//
// Purpose-
//       Set the Handler.
//
//----------------------------------------------------------------------------
void
   Handler::setHandler(             // Set the Handler
     Handler*          handler)     // The Handler
{
   this->handler= handler;
}

#endif // HANDLER_I_INCLUDED
