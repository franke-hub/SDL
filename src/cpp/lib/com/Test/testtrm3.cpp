//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Testtrm3.cpp
//
// Purpose-
//       Test Keyboard/Screen constructor/destructor.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <com/Color.h>
#include <com/Debug.h>
#include <com/Handler.h>
#include "com/Keyboard.h"
#include "com/TextScreen.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TESTTRM3" // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define REPEAT_COUNT              1 // Number of times to construct/destroy

//----------------------------------------------------------------------------
// Internal classes
//----------------------------------------------------------------------------
class MyHandler : public Handler {
protected:
virtual void
   handleError(void)                // Error handler
{
   tracef( "Error(%d) handled\n", getIdent() );
}
}; // class MyHandler

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static MyHandler       handler;     // Handler  object

//----------------------------------------------------------------------------
//
// Subroutine-
//       doKeyboard
//
// Purpose-
//       Construct and deconstruct a keyboard object.
//
//----------------------------------------------------------------------------
static void
   doKeyboard( void )               // Construct/deconstruct a keyboard
{
   Keyboard        keyboard;        // Build a Keyboard object

   keyboard.setHandler(&handler);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       doScreen
//
// Purpose-
//       Construct and deconstruct a TextScreen object.
//
//----------------------------------------------------------------------------
static void
   doScreen( void )                 // Construct/deconstruct a keyboard
{
   TextScreen      screen;          // Build a TextScreen object

   screen.setHandler(&handler);
   screen.setAttribute(VGAColor::White, VGAColor::Blue);
   screen.clearScreen();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       doBoth
//
// Purpose-
//       Construct and deconstruct a TextScreen object.
//
//----------------------------------------------------------------------------
static void
   doBoth( void )                   // Construct/deconstruct a keyboard
{
   Keyboard        keyboard;        // Build a Keyboard object
   TextScreen      screen;          // Build a TextScreen object

   keyboard.setHandler(&handler);
   screen.setHandler(&handler);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(int, char**)                // Mainline code
//   int             argc,          // Argument count
//   char*           argv[])        // Argument array
{
   int               i;

   //-------------------------------------------------------------------------
   // Initialize debugging
   //-------------------------------------------------------------------------
   debugSetIntensiveMode();

   //-------------------------------------------------------------------------
   // Keyboard test
   //-------------------------------------------------------------------------
   debugf("Keyboard\n");
   for(i=0; i<REPEAT_COUNT; i++)
   {
     doKeyboard();
   }

   //-------------------------------------------------------------------------
   // Screen test
   //-------------------------------------------------------------------------
   debugf("Screen\n");
   for(i=0; i<REPEAT_COUNT; i++)
   {
     tracef("Before Screen[%2d]\n", i);
     doScreen();
     tracef("After  Screen[%2d]\n", i);
   }

   //-------------------------------------------------------------------------
   // Combined test
   //-------------------------------------------------------------------------
   debugf("Combined\n");
   for(i=0; i<REPEAT_COUNT; i++)
   {
     doBoth();
   }

   //-------------------------------------------------------------------------
   // Testing complete
   //-------------------------------------------------------------------------
   debugf("Done!\n");

   return 0;
}

