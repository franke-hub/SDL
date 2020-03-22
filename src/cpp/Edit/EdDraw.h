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
//       EdDraw.h
//
// Purpose-
//       Editor: Base class for display objects.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#ifndef EDDRAW_H_INCLUDED
#define EDDRAW_H_INCLUDED

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Terminal;

//----------------------------------------------------------------------------
//
// Class-
//       EdDraw
//
// Purpose-
//       Editor display object.
//
//----------------------------------------------------------------------------
class EdDraw {                      // Editor display object.
//----------------------------------------------------------------------------
// EdDraw::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum ColorSet                       // Color set
{  CS_BG= 0                         // Background color
,  CS_FG                            // Foreground color
,  CS_MAX                           // Number color sets
}; // enum ColorSet

enum MsgFsm                         // Message/status line state
{  MSG_NONE                         // No message
,  MSG_INFO                         // Informational message
,  MSG_WARN                         // Warning message
,  MSG_ERROR                        // Error message
,  MSG_REPLY                        // Reply message
,  MSG_ALARM                        // Alarm message
}; // enum MsgFsm

enum ReshowType                     // Reshow type (deferred)
{  RESHOW_ALL                       // Reshow everything (unconditionally)
,  RESHOW_BUF                       // Reshow the buffer
,  RESHOW_CSR= (-1)                 // Reshow the cursor
}; // enum ReshowType

//----------------------------------------------------------------------------
// EdDraw::Constructors
//----------------------------------------------------------------------------
public:
   ~EdDraw( void );                 // Destructor
   EdDraw(                          // Constructor
     Terminal*         terminal);   // Terminal

//----------------------------------------------------------------------------
// EdDraw::Methods
//----------------------------------------------------------------------------
public:
inline Terminal*                    // The Terminal
   getTerminal( void );             // Get Terminal

//----------------------------------------------------------------------------
// EdDraw::Attributes
//----------------------------------------------------------------------------
protected:
   Terminal*           const terminal; // -> Terminal
}; // class EdDraw

#include "EdDraw.i"

#endif // EDDRAW_H_INCLUDED
