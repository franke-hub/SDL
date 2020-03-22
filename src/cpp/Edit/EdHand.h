//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2016 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdHand.h
//
// Purpose-
//       Editor: Terminal exception handler.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#ifndef EDHAND_H_INCLUDED
#define EDHAND_H_INCLUDED

#ifndef EDITOR_H_INCLUDED
#include "Editor.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       EdHand
//
// Purpose-
//       Define editor exception handler.
//
//----------------------------------------------------------------------------
class EdHand : public Handler       // Editor exception handler
{
//----------------------------------------------------------------------------
// EdHand::Constructors
//----------------------------------------------------------------------------
public:
   ~EdHand( void );                 // Destructor

   EdHand(                          // Constructor
     Terminal*         terminal);   // -> terminal

//----------------------------------------------------------------------------
// EdHand::Methods
//----------------------------------------------------------------------------
public:
virtual void
   handleAbort( void );

virtual void
   handleError( void );

virtual void
   handleEvent( void );

//----------------------------------------------------------------------------
// EdHand::Attributes
//----------------------------------------------------------------------------
protected:
   Terminal*         terminal;      // -> terminal
}; // class EdHand

#endif // EDHAND_H_INCLUDED
