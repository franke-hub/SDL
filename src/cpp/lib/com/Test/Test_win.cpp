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
//       Test_win.cpp
//
// Purpose-
//       Test Windows functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include <com/Debug.h>
#include <com/Terminal.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TEST_WIN" // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define ESC                      27 // ASCII code for Escape

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Terminal        terminal;    // Terminal object

//----------------------------------------------------------------------------
// External routines without prototypes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   //-------------------------------------------------------------------------
   // Initialize debugging
   //-------------------------------------------------------------------------
   debugSetIntensiveMode();

   //-------------------------------------------------------------------------
   // Open the keyboard and screen
   //-------------------------------------------------------------------------
   terminal.construct();
   terminal.setDattr(Terminal::WHITE, Terminal::BLUE);

   //-------------------------------------------------------------------------
   // Initial screen write/read
   //-------------------------------------------------------------------------
   terminal.clearScreen();
   tracef("Initial screen write\n");
   terminal.wr(0, "This is the initial screen write");
   terminal.rd();

   //-------------------------------------------------------------------------
   // Windows keyboard test
   //-------------------------------------------------------------------------
#if 1
{{{{
   INPUT_RECORD      inpRecord[16];
   DWORD             eventCount;
   unsigned int      inpCode;
   unsigned int      inpScan;
   DWORD             inpState;
   HANDLE            keyH;
   int               c;             // Last character read
   int               p;             // Next to last character read

   terminal.clearScreen();
   keyH= GetStdHandle(STD_INPUT_HANDLE);
// SetConsoleMode(keyH, ENABLE_WINDOW_INPUT);

   p= '\0';
   c= '\0';
   for(;;)
   {
     ReadConsoleInput(
         keyH,                      // input buffer handle
         inpRecord,                 // buffer to read into
         1,                         // size of read buffer
         &eventCount);              // number of records read

     if (inpRecord[0].EventType == KEY_EVENT)
     {
       inpCode=  inpRecord[0].Event.KeyEvent.uChar.UnicodeChar;
       inpScan=  inpRecord[0].Event.KeyEvent.wVirtualScanCode;
       inpState= inpRecord[0].Event.KeyEvent.dwControlKeyState;

       if (inpRecord[0].Event.KeyEvent.bKeyDown)
       {
         p= c;
         c= inpCode;
       }

       terminal.physicalXY(0,0);
       terminal.printf("%c ", isprint(inpCode)? inpCode : '.');
       terminal.printf("EC(%.1d) KC(%.4x) SC(%.4x) RC(%2d) ",
                       eventCount, inpCode, inpScan,
                       inpRecord[0].Event.KeyEvent.wRepeatCount);
       if (inpRecord[0].Event.KeyEvent.bKeyDown == TRUE)
         terminal.printf("DOWN ");
       else if (inpRecord[0].Event.KeyEvent.bKeyDown == FALSE)
         terminal.printf("UP   ");
       else
         terminal.printf("%.8lX ", (long)inpRecord[0].Event.KeyEvent.bKeyDown);

       terminal.printf("STATE(%.8lx)\n",
                       (long)inpState);

       tracef("%c ", isprint(inpCode)? inpCode : '.');
       tracef("EC(%.1d) KC(%.4x) SC(%.4x) RC(%2d) ",
              eventCount, inpCode, inpScan,
              inpRecord[0].Event.KeyEvent.wRepeatCount);
       tracef("STATE(%.8lx) ",
              (long)inpState);
       if (inpRecord[0].Event.KeyEvent.bKeyDown == TRUE)
         tracef("DOWN ");
       else if (inpRecord[0].Event.KeyEvent.bKeyDown == FALSE)
         tracef("UP   ");
       else
         tracef("%.8lX ", (long)inpRecord[0].Event.KeyEvent.bKeyDown);
       tracef("\n");
     }
     if (c == ESC && p == ESC)
       break;
   }
}}}}
#endif // Windows keyboard test

   //-------------------------------------------------------------------------
   // Testing complete
   //-------------------------------------------------------------------------
   tracef("Done!\n");
   printf("Testing complete\n");

   return 0;
}

