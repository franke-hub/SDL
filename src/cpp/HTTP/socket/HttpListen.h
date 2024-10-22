//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpListen.h
//
// Purpose-
//       HTTP Listen object.
//
// Last change date-
//       2024/10/08
//
//----------------------------------------------------------------------------
#ifndef _LISTEN_H_INCLUDED
#define _LISTEN_H_INCLUDED

#include <pub/Thread.h>             // For pub::Thread, base class

//----------------------------------------------------------------------------
//
// Class-
//       Listener
//
// Purpose-
//       The Listener Thread
//
//----------------------------------------------------------------------------
class Listener : public pub::Thread { // The Listener Thread
bool                   operational= false;

public:
   Listener( void );                // Constructor

virtual
   ~Listener( void );               // Destructor

//----------------------------------------------------------------------------
// Run the Listener Thread
//----------------------------------------------------------------------------
virtual void
   run( void );                     // Run the Listener Thread
}; // class Listener
typedef Listener       HttpListen;  // Alias
#endif // _LISTEN_H_INCLUDED
