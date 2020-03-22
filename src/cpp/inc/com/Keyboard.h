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
//       Keyboard.h
//
// Purpose-
//       Keyboard control functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED

#ifndef HANDLER_H_INCLUDED
#include "Handler.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Keyboard
//
// Purpose-
//       System-dependent keyboard function.
//
//----------------------------------------------------------------------------
class Keyboard : virtual public Handler { // Keyboard functions
//----------------------------------------------------------------------------
// Keyboard::Attributes
//----------------------------------------------------------------------------
private:
void*                  attr;        // Hidden attributes

//----------------------------------------------------------------------------
// Keyboard::Constructors
//----------------------------------------------------------------------------
public:
   ~Keyboard( void );               // Destructor
   Keyboard( void );                // Constructor

private:                            // Bitwise copy prohibited
   Keyboard(const Keyboard&);       // Disallowed copy constructor
   Keyboard& operator=(const Keyboard&); // Disallowed assignment operator

//----------------------------------------------------------------------------
//
// Method-
//       Keyboard::ifInsertKey
//
// Purpose-
//       Determine whether the Insert key is locked.
//
//----------------------------------------------------------------------------
public:
int                                 // TRUE if insert lock
   ifInsertKey( void );             // Is insert key locked?

//----------------------------------------------------------------------------
//
// Method-
//       Keyboard::ifScrollKey
//
// Purpose-
//       Determine whether the Scroll key is locked.
//
//----------------------------------------------------------------------------
public:
int                                 // TRUE if scroll lock
   ifScrollKey( void );             // Is scroll key locked?

//----------------------------------------------------------------------------
//
// Method-
//       Keyboard::poll( void )
//
// Purpose-
//       Determine whether a keypress is available.
//
//----------------------------------------------------------------------------
public:
int                                 // TRUE if character available
   poll(                            // Is keypress present?
     unsigned          delay= 0);   // Delay in milliseconds

//----------------------------------------------------------------------------
//
// Method-
//       Keyboard::rd( void )
//
// Purpose-
//       Read one character from the keyboard, waiting if none is available.
//
//----------------------------------------------------------------------------
public:
int                                 // Keyboard character
   rd( void );                      // Read character from keyboard
}; // class Keyboard

#endif // KEYBOARD_H_INCLUDED
