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
//       Terminal.h
//
// Purpose-
//       Terminal control.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef TERMINAL_H_INCLUDED
#define TERMINAL_H_INCLUDED

#include "Keyboard.h"
#include "TextScreen.h"

//----------------------------------------------------------------------------
//
// Class-
//       Terminal
//
// Purpose-
//       Terminal (Keyboard and Screen) I/O control.
//
//----------------------------------------------------------------------------
class Terminal : public Keyboard, public TextScreen  { // Text terminal control
//----------------------------------------------------------------------------
// Terminal::Attributes
//----------------------------------------------------------------------------
private:
   // None defined

//----------------------------------------------------------------------------
// TextScreen::Enumerations
//----------------------------------------------------------------------------
public:
enum Error                          // Error values
{
   ErrorNone= 0,                    // No error
   ErrorPosition= 101,              // Invalid position
   ErrorColor                       // Invalid color
}; // enum Error

enum Event                          // Event values
{
   EventNone= 0,                    // No event
   EventResize                      // Window resize event
}; // enum Event

//----------------------------------------------------------------------------
// Terminal::Constructors
//----------------------------------------------------------------------------
public:
   ~Terminal( void );               // Destructor
   Terminal( void );                // Constructor

private:
   // class Terminal may neither be copied nor assigned
   Terminal(const Terminal&);       // Private copy constructor
   Terminal& operator=(const Terminal&);// Private assignment operator
}; // class Terminal

#endif // TERMINAL_H_INCLUDED
